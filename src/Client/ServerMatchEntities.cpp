// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ServerMatchEntities.hpp>
#include <Nazara/Core/Clock.hpp>
#include <Nazara/Core/Primitive.hpp>
#include <Nazara/Graphics/Billboard.hpp>
#include <Nazara/Graphics/Model.hpp>
#include <Nazara/Graphics/ParticleFunctionController.hpp>
#include <Nazara/Graphics/ParticleFunctionGenerator.hpp>
#include <Nazara/Graphics/ParticleFunctionRenderer.hpp>
#include <Nazara/Graphics/ParticleMapper.hpp>
#include <Nazara/Graphics/ParticleStruct.hpp>
#include <Nazara/Graphics/TextSprite.hpp>
#include <Nazara/Renderer/DebugDrawer.hpp>
#include <Nazara/Renderer/Renderer.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <Nazara/Utility/StaticMesh.hpp>
#include <Nazara/Utility/VertexMapper.hpp>
#include <NDK/Components.hpp>
#include <Client/ClientApplication.hpp>
#include <Client/Components/SoundEmitterComponent.hpp>
#include <iostream>

namespace ewn
{
	static constexpr bool showServerGhosts = false;

	ServerMatchEntities::ServerMatchEntities(ClientApplication* app, ServerConnection* server, Ndk::WorldHandle world) :
	m_jitterBuffer(m_jitterBufferData.begin(), m_jitterBufferData.end()),
	m_world(std::move(world)),
	m_app(app),
	m_server(server),
	m_stateHandlingEnabled(true),
	m_correctionAccumulator(0.f),
	m_snapshotUpdateAccumulator(0.f)
	{
		m_snapshotDelay = m_jitterBuffer.size() * 1000 / 30 /* + ping? */;

		m_onArenaParticleSystemsSlot.Connect(server->OnArenaParticleSystems, this, &ServerMatchEntities::OnArenaParticleSystems);
		m_onArenaPrefabsSlot.Connect(server->OnArenaPrefabs, this, &ServerMatchEntities::OnArenaPrefabs);
		m_onArenaSoundsSlot.Connect(server->OnArenaSounds, this,   &ServerMatchEntities::OnArenaSounds);
		m_onArenaStateSlot.Connect(server->OnArenaState, this,     &ServerMatchEntities::OnArenaState);
		m_onCreateEntitySlot.Connect(server->OnCreateEntities, this, &ServerMatchEntities::OnCreateEntities);
		m_onDeleteEntitySlot.Connect(server->OnDeleteEntities, this, &ServerMatchEntities::OnDeleteEntities);
		m_onInstantiateParticleSystemSlot.Connect(server->OnInstantiateParticleSystem, this, &ServerMatchEntities::OnInstantiateParticleSystem);
		m_onPlaySoundSlot.Connect(server->OnPlaySound, this,       &ServerMatchEntities::OnPlaySound);

		FillVisualEffectFactory();

		// Listen to debug state
		if constexpr (showServerGhosts)
		{
			m_debugStateSocket.Create(Nz::NetProtocol_IPv4);
			m_debugStateSocket.Bind(2050);

			m_debugStateSocket.EnableBlocking(false);
		}

		std::random_device rd;
		m_randomGenerator.seed(rd());
	}

	ServerMatchEntities::~ServerMatchEntities()
	{
		for (const auto& spaceshipData : m_serverEntities)
		{
			if (spaceshipData.debugGhostEntity)
				spaceshipData.debugGhostEntity->Kill();

			if (spaceshipData.entity)
				spaceshipData.entity->Kill();

			if (spaceshipData.textEntity)
				spaceshipData.textEntity->Kill();
		}
	}

	void ServerMatchEntities::Update(float elapsedTime)
	{
		HandlePlayingSounds();

		if (m_stateHandlingEnabled)
		{
			Nz::UInt64 serverTime = m_server->EstimateServerTime();
			if (!m_jitterBuffer.empty() && serverTime >= m_jitterBuffer.front().applyTime)
			{
				ApplySnapshot(m_jitterBuffer.front());
				m_jitterBuffer.pop_back();
			}
		}

		constexpr float errorCorrectionInterval = 1.f / 60.f;

		m_correctionAccumulator += elapsedTime;
		while (m_correctionAccumulator >= errorCorrectionInterval)
		{
			m_correctionAccumulator -= errorCorrectionInterval;

			for (auto& spaceshipData : m_serverEntities)
			{
				if (!spaceshipData.entity)
					continue;

				auto& entityNode = spaceshipData.entity->GetComponent<Ndk::NodeComponent>();
				auto& entityPhys = spaceshipData.entity->GetComponent<Ndk::PhysicsComponent3D>();

				//if (spaceshipData.entity->GetId() == 9)
				//	std::cout << "#" << spaceshipData.entity->GetId() << ": " << entityPhys.GetLinearVelocity() << " " << entityPhys.GetAngularVelocity() << std::endl;

				entityNode.SetPosition(entityPhys.GetPosition() + spaceshipData.positionError);
				entityNode.SetRotation(entityPhys.GetRotation() * spaceshipData.rotationError);

				//spaceshipData.positionError += entityPhys.GetLinearVelocity();

				/*if (spaceshipData.entity->GetId() == 9)
				{
					std::cout << entityPhys.GetLinearVelocity().GetLength() << std::endl;
					std::cout << spaceshipData.positionError << std::endl;
				}*/

				spaceshipData.positionError = Nz::Lerp(spaceshipData.positionError, Nz::Vector3f::Zero(), 0.1f);
				spaceshipData.rotationError = Nz::Quaternionf::Slerp(spaceshipData.rotationError, Nz::Quaternionf::Identity(), 0.1f);

				// Avoid denormals
				for (std::size_t i = 0; i < 3; ++i)
				{
					if (Nz::NumberEquals(spaceshipData.positionError[i], 0.f, 0.001f))
						spaceshipData.positionError[i] = 0.f;
				}

				/*for (std::size_t i = 0; i < 4; ++i)
				{
					if (Nz::NumberEquals(spaceshipData.rotationError[i], 0.f, 0.001f))
						spaceshipData.rotationError[i] = 0.f;
				}*/
			}
		}

		/*if constexpr (showServerGhosts)
		{
			Nz::NetPacket packet;
			if (m_debugStateSocket.ReceivePacket(&packet, nullptr))
			{
				Packets::ArenaState arenaState;
				PacketSerializer serializer(packet, false);
				Packets::Serialize(serializer, arenaState);

				for (auto& serverData : arenaState.entities)
				{
					// Since we're using a different channel for debug purpose, we may receive information about a spaceship we're not aware yet
					if (!IsServerEntityValid(serverData.id))
						continue;

					ServerEntity& entityData = GetServerEntity(serverData.id);

					// Ensure ghost entity existence
					if (!entityData.debugGhostEntity)
						entityData.debugGhostEntity = m_debugTemplateEntity->Clone();

					auto& ghostNode = entityData.debugGhostEntity->GetComponent<Ndk::NodeComponent>();
					ghostNode.SetPosition(serverData.position);
					ghostNode.SetRotation(serverData.rotation);
				}
			}
		}*/
	}

	void ServerMatchEntities::FillVisualEffectFactory()
	{
		// Earth
		m_visualEffectFactory["earth"] = [](ClientApplication* app, const Ndk::EntityHandle& entity)
		{
			const std::string& assetsFolder = app->GetConfig().GetStringOption("AssetsFolder");

			Nz::MeshRef earthMesh = Nz::Mesh::New();
			earthMesh->CreateStatic();
			earthMesh->BuildSubMesh(Nz::Primitive::UVSphere(50.f, 40, 40));

			Nz::MaterialRef earthMaterial = Nz::Material::New();
			earthMaterial->SetDiffuseMap(assetsFolder + "/earth/earthmap1k.jpg");
			earthMaterial->SetShader("PhongLighting");

			Nz::ModelRef earthModel = Nz::Model::New();
			earthModel->SetMesh(earthMesh);
			earthModel->SetMaterial(0, earthMaterial);

			entity->GetComponent<Ndk::GraphicsComponent>().Attach(earthModel);
		};

		// Scene light
		m_visualEffectFactory["light"] = [](ClientApplication* /*app*/, const Ndk::EntityHandle& entity)
		{
			entity->AddComponent<Ndk::LightComponent>(Nz::LightType_Directional);
		};

		// Projectile (laser)
		m_visualEffectFactory["plasmabeam"] = [](ClientApplication* app, const Ndk::EntityHandle& entity)
		{
			const std::string& assetsFolder = app->GetConfig().GetStringOption("AssetsFolder");

			Nz::TextureSampler diffuseSampler;
			diffuseSampler.SetAnisotropyLevel(4);
			diffuseSampler.SetWrapMode(Nz::SamplerWrap_Repeat);

			Nz::MaterialRef material = Nz::Material::New("Translucent3D");
			material->SetDiffuseMap(assetsFolder + "/weapons/LaserBeam.png");
			material->SetDiffuseSampler(diffuseSampler);
			material->SetEmissiveMap(assetsFolder + "/weapons/LaserBeam.png");
			material->SetShader("PhongLighting");

			Nz::SpriteRef laserSprite1 = Nz::Sprite::New();
			laserSprite1->SetMaterial(material);
			laserSprite1->SetOrigin(Nz::Vector2f(0.f, 0.5f));
			laserSprite1->SetSize({ 5.f, 1.f });
			laserSprite1->SetTextureCoords(Nz::Rectf(0.f, 0.f, 1.f, 1.f));

			Nz::SpriteRef laserSprite2 = Nz::Sprite::New(*laserSprite1);

			auto& gfxComponent = entity->GetComponent<Ndk::GraphicsComponent>();

			gfxComponent.Attach(laserSprite1, Nz::Matrix4f::Transform(Nz::Vector3f::Backward() * 2.5f, Nz::EulerAnglesf(0.f, 90.f, 0.f)));
			gfxComponent.Attach(laserSprite2, Nz::Matrix4f::Transform(Nz::Vector3f::Backward() * 2.5f, Nz::EulerAnglesf(90.f, 90.f, 0.f)));
		};

		// Projectile (torpedo)
		m_visualEffectFactory["torpedo"] = [](ClientApplication* app, const Ndk::EntityHandle& entity)
		{
			const std::string& assetsFolder = app->GetConfig().GetStringOption("AssetsFolder");

			auto& gfxComponent = entity->GetComponent<Ndk::GraphicsComponent>();

			Nz::MaterialRef flareMaterial = Nz::Material::New("Translucent3D");
			flareMaterial->SetDiffuseMap(assetsFolder + "/weapons/flare1.png");
			flareMaterial->SetShader("Basic");

			Nz::BillboardRef billboard = Nz::Billboard::New();
			billboard->SetMaterial(flareMaterial);
			billboard->SetSize(billboard->GetSize() * 0.025f);

			gfxComponent.Attach(billboard);
		};
	}

	void ServerMatchEntities::HandlePlayingSounds()
	{
		for (auto it = m_playingSounds.begin(); it != m_playingSounds.end();)
		{
			if (!it->IsPlaying())
				it = m_playingSounds.erase(it);
			else
				++it;
		}
	}

	void ServerMatchEntities::OnArenaPrefabs(ServerConnection* server, const Packets::ArenaPrefabs& arenaPrefabs)
	{
		m_prefabs.erase(m_prefabs.begin() + arenaPrefabs.startId, m_prefabs.end());

		const std::string& assetsFolder = server->GetApp().GetConfig().GetStringOption("AssetsFolder");

		const NetworkStringStore& networkStringStore = server->GetNetworkStringStore();
		for (const auto& prefab : arenaPrefabs.prefabs)
		{
			const Ndk::EntityHandle& entity = m_world->CreateEntity();
			entity->Disable();

			entity->AddComponent<Ndk::NodeComponent>();

			auto& graphicsComponent = entity->AddComponent<Ndk::GraphicsComponent>();
			for (const auto& modelPiece : prefab.models)
			{
				Nz::Matrix4f transformMatrix = Nz::Matrix4f::Transform(modelPiece.position, modelPiece.rotation, modelPiece.scale);

				// TODO: Load it once for every path
				std::string filePath = assetsFolder + '/' + networkStringStore.GetString(modelPiece.modelId);

				/*Nz::MeshParams collisionParams;
				collisionParams.animated = false;
				collisionParams.center = true;
				collisionParams.optimizeIndexBuffers = false;
				collisionParams.storage = Nz::DataStorage_Software;
				collisionParams.matrix = Nz::Matrix4f::Transform(modelPiece.position, modelPiece.rotation, Nz::Vector3f(modelPiece.scale));

				Nz::Mesh collisionMesh;
				if (collisionMesh.LoadFromFile(filePath, collisionParams))
				{
					std::vector<Nz::Vector3f> vertices;
					for (std::size_t i = 0; i < collisionMesh.GetSubMeshCount(); ++i)
					{
						Nz::VertexMapper vertexMapper(collisionMesh.GetSubMesh(i), Nz::BufferAccess_ReadOnly);
						Nz::SparsePtr<Nz::Vector3f> subMeshVertices = vertexMapper.GetComponentPtr<Nz::Vector3f>(Nz::VertexComponent_Position);

						Nz::UInt32 vertexCount = vertexMapper.GetVertexCount();
						vertices.reserve(vertices.size() + vertexCount);
						for (Nz::UInt32 i = 0; i < vertexCount; ++i)
							vertices.push_back(subMeshVertices[i]);
					}

					auto collider = Nz::ConvexCollider3D::New(vertices.data(), vertices.size(), 0.01f);
					entity->AddComponent<Ndk::CollisionComponent3D>(collider);
					entity->AddComponent<Ndk::DebugComponent>(Ndk::DebugDraw::Collider3D | Ndk::DebugDraw::GraphicsAABB);
				}*/

				if (Nz::ModelRef model = Nz::ModelManager::Get(filePath))
					graphicsComponent.Attach(model, transformMatrix);
				else
					std::cerr << "Failed to load " << filePath << std::endl;
			}

			auto& physComponent = entity->AddComponent<Ndk::PhysicsComponent3D>();
			physComponent.EnableNodeSynchronization(false);
			physComponent.SetMass(10.f);
			physComponent.SetAngularDamping(Nz::Vector3f(0.f));
			physComponent.SetLinearDamping(0.f);

			for (const auto& sound : prefab.sounds)
			{
				if (!entity->HasComponent<SoundEmitterComponent>())
					entity->AddComponent<SoundEmitterComponent>();

				SoundEmitterComponent& soundEmitter = entity->GetComponent<SoundEmitterComponent>();
				soundEmitter.EnableLooping(true);
				soundEmitter.EnableSpatialization(true);
				soundEmitter.SetBuffer(m_soundLibrary[sound.soundId]);
				soundEmitter.SetMinDistance(20.f);

				// TODO: Support multiple sounds (and relative positioning)
				break;
			}

			for (const auto& visualEffect : prefab.visualEffects)
			{
				auto visualEffectIt = m_visualEffectFactory.find(networkStringStore.GetString(visualEffect.effectNameId));
				assert(visualEffectIt != m_visualEffectFactory.end());

				visualEffectIt->second(m_app, entity);
			}

			m_prefabs.emplace_back(entity);
		}
	}

	void ServerMatchEntities::OnArenaParticleSystems(ServerConnection* server, const Packets::ArenaParticleSystems& arenaParticleSystems)
	{
		const std::string& assetsFolder = server->GetApp().GetConfig().GetStringOption("AssetsFolder");

		const NetworkStringStore& networkStringStore = server->GetNetworkStringStore();

		m_particleSystems.erase(m_particleSystems.begin() + arenaParticleSystems.startId, m_particleSystems.end());
		for (const auto& packetParticleSystem : arenaParticleSystems.particleSystems)
		{
			ParticleSystem& particleSystem = m_particleSystems.emplace_back();

			for (const auto& packetParticleGroup : packetParticleSystem.particleGroups)
			{
				ParticleSystem::ParticleGroup& particleGroup = particleSystem.particleGroups.emplace_back();

				const std::string& particleGroupName = networkStringStore.GetString(packetParticleGroup.particleGroupNameId);

				particleGroup.particleGroup = m_world->CreateEntity();

				// Warning: you are entering the ugly zone
				class AlphaController : public Nz::ParticleController
				{
					public:
						AlphaController(float alphaLostPerSeconds) :
						m_alphaLoss(alphaLostPerSeconds),
						m_alphaCounter(0.f)
						{
						}

						void Apply(Nz::ParticleGroup& group, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId, float elapsedTime) override
						{
							auto particleColor = mapper.GetComponentPtr<Nz::Color>(Nz::ParticleComponent_Color);

							m_alphaCounter += m_alphaLoss * elapsedTime;

							Nz::UInt8 alphaLoss = static_cast<Nz::UInt8>(m_alphaCounter);
							if (alphaLoss == 0)
								return;

							m_alphaCounter -= alphaLoss;

							for (unsigned int i = startId; i <= endId; ++i)
							{
								if (particleColor[i].a > alphaLoss)
									particleColor[i].a -= alphaLoss;
								else
									group.KillParticle(i);
							}
						}

					private:
						float m_alphaLoss;
						float m_alphaCounter;
				};

				class LifeController : public Nz::ParticleController
				{
					public:
						void Apply(Nz::ParticleGroup& group, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId, float elapsedTime) override
						{
							auto particleLife = mapper.GetComponentPtr<float>(Nz::ParticleComponent_Life);

							for (unsigned int i = startId; i <= endId; ++i)
							{
								particleLife[i] -= elapsedTime;
								if (particleLife[i] <= 0.f)
									group.KillParticle(i);
							}
						}
				};

				class GrowthController : public Nz::ParticleController
				{
					public:
						GrowthController(float growthFactor) :
						m_growthFactor(growthFactor)
						{
						}

						void Apply(Nz::ParticleGroup& group, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId, float elapsedTime) override
						{
							auto particleSize = mapper.GetComponentPtr<Nz::Vector2f>(Nz::ParticleComponent_Size);

							for (unsigned int i = startId; i <= endId; ++i)
								particleSize[i] += Nz::Vector2f(elapsedTime) * m_growthFactor;
						}

					private:
						float m_growthFactor;
				};

				class VelocityController : public Nz::ParticleController
				{
					public:
						void Apply(Nz::ParticleGroup& group, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId, float elapsedTime) override
						{
							auto particlePos = mapper.GetComponentPtr<Nz::Vector3f>(Nz::ParticleComponent_Position);
							auto particleVel = mapper.GetComponentPtr<Nz::Vector3f>(Nz::ParticleComponent_Velocity);

							for (unsigned int i = startId; i <= endId; ++i)
								particlePos[i] += particleVel[i] * elapsedTime;
						}
				};

				if (particleGroupName == "explosion_flare")
				{
					auto& entityGroup = particleGroup.particleGroup->AddComponent<Ndk::ParticleGroupComponent>(10'000, Nz::ParticleLayout_Billboard);

					entityGroup.AddController(std::make_unique<AlphaController>(1000.f).release());
					entityGroup.AddController(std::make_unique<LifeController>().release());
					entityGroup.AddController(std::make_unique<VelocityController>().release());

					// Temporary fix for flare orientation depending on camera
					/*entityGroup.AddController(Nz::ParticleFunctionController::New([this](Nz::ParticleGroup& group, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId, float elapsedTime)
					{
						auto particleRotation = mapper.GetComponentPtr<float>(Nz::ParticleComponent_Rotation);
						auto particleVel = mapper.GetComponentPtr<Nz::Vector3f>(Nz::ParticleComponent_Velocity);

						for (unsigned int i = startId; i <= endId; ++i)
						{
							Nz::Vector3f velocity = particleVel[i];
							velocity.Normalize();

							particleRotation[i] = Nz::Vector3f::DotProduct(Nz::Vector3f::Up(), velocity) * 180.f;
						}
					}));*/

					entityGroup.AddGenerator(Nz::ParticleFunctionGenerator::New([this](Nz::ParticleGroup& /*group*/, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId)
					{
						auto particleColor = mapper.GetComponentPtr<Nz::Color>(Nz::ParticleComponent_Color);
						auto particleLife = mapper.GetComponentPtr<float>(Nz::ParticleComponent_Life);
						auto particleRotation = mapper.GetComponentPtr<float>(Nz::ParticleComponent_Rotation);
						auto particleSize = mapper.GetComponentPtr<Nz::Vector2f>(Nz::ParticleComponent_Size);
						auto particleVel = mapper.GetComponentPtr<Nz::Vector3f>(Nz::ParticleComponent_Velocity);

						std::uniform_real_distribution<float> lifeDis(2.f, 5.f);
						std::uniform_real_distribution<float> normalDis(-1.f, 1.f);
						std::uniform_real_distribution<float> velocityDis(20.f, 50.f);
						for (unsigned int i = startId; i <= endId; ++i)
						{
							Nz::Vector3f normal(normalDis(m_randomGenerator), normalDis(m_randomGenerator), normalDis(m_randomGenerator));
							normal.Normalize();

							particleColor[i] = Nz::Color::White;
							particleLife[i] = lifeDis(m_randomGenerator);
							particleRotation[i] = Nz::Vector3f::DotProduct(Nz::Vector3f::Up(), normal) * 180.f;
							particleSize[i] = Nz::Vector2f(64.f, 64.f) / 50.f;
							particleVel[i] = normal * velocityDis(m_randomGenerator);
						}
					}));

					Nz::MaterialRef flareMaterial = Nz::Material::New("Translucent3D");
					flareMaterial->SetDiffuseMap(assetsFolder + "particles/spark.png");

					entityGroup.SetRenderer(Nz::ParticleFunctionRenderer::New([flareMaterial](const Nz::ParticleGroup& /*group*/, const Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId, Nz::AbstractRenderQueue* renderQueue)
					{
						auto particlePos = mapper.GetComponentPtr<Nz::Vector3f>(Nz::ParticleComponent_Position) + startId;
						auto particleColor = mapper.GetComponentPtr<Nz::Color>(Nz::ParticleComponent_Color) + startId;
						auto particleRotation = mapper.GetComponentPtr<float>(Nz::ParticleComponent_Rotation) + startId;
						auto particleSize = mapper.GetComponentPtr<Nz::Vector2f>(Nz::ParticleComponent_Size) + startId;

						renderQueue->AddBillboards(0, flareMaterial, endId - startId + 1, Nz::Recti(-1, -1), particlePos, particleSize, particleRotation, particleColor);
					}));

					particleGroup.instantiate = [this](const Ndk::EntityHandle& group, const Nz::Vector3f& position, const Nz::Quaternionf& /*rotation*/)
					{
						/*std::uniform_int_distribution<unsigned int> particleCountDis(20, 50);
						unsigned int particleCount = particleCountDis(m_randomGenerator);

						auto& entityGroup = group->GetComponent<Ndk::ParticleGroupComponent>();
						Nz::ParticleStruct_Billboard* particles = static_cast<Nz::ParticleStruct_Billboard*>(entityGroup.GenerateParticles(particleCount));
						if (!particles)
							return;

						for (unsigned int i = 0; i < particleCount; ++i)
							particles[i].position = position;*/
					};
				}
				else if (particleGroupName == "explosion_fire")
				{
					auto& entityGroup = particleGroup.particleGroup->AddComponent<Ndk::ParticleGroupComponent>(10'000, Nz::ParticleLayout_Billboard);

					//entityGroup.AddController(std::make_unique<AlphaController>(150.f).release());
					entityGroup.AddController(std::make_unique<LifeController>().release());
					entityGroup.AddController(std::make_unique<GrowthController>(2.f).release());
					entityGroup.AddController(std::make_unique<VelocityController>().release());

					entityGroup.AddController(Nz::ParticleFunctionController::New([this](Nz::ParticleGroup& group, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId, float elapsedTime)
					{
						auto colorPtr = mapper.GetComponentPtr<Nz::Color>(Nz::ParticleComponent_Color);
						auto lifePtr = mapper.GetComponentPtr<float>(Nz::ParticleComponent_Life);

						for (unsigned int i = startId; i <= endId; ++i)
							colorPtr[i].a = static_cast<Nz::UInt8>(Nz::Clamp(lifePtr[i] * 255.f, 0.f, 255.f));
					}));

					entityGroup.AddGenerator(Nz::ParticleFunctionGenerator::New([this](Nz::ParticleGroup& /*group*/, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId)
					{
						auto particleColor = mapper.GetComponentPtr<Nz::Color>(Nz::ParticleComponent_Color);
						auto particleRotation = mapper.GetComponentPtr<float>(Nz::ParticleComponent_Rotation);
						auto particleSize = mapper.GetComponentPtr<Nz::Vector2f>(Nz::ParticleComponent_Size);
						auto particleVel = mapper.GetComponentPtr<Nz::Vector3f>(Nz::ParticleComponent_Velocity);
						auto particleLife = mapper.GetComponentPtr<float>(Nz::ParticleComponent_Life);

						std::uniform_real_distribution<float> normalDis(-1.f, 1.f);
						std::uniform_real_distribution<float> lifeDis(0.5f, 1.f);
						std::uniform_real_distribution<float> rotDis(-180.f, 180.f);
						std::uniform_real_distribution<float> sizeDis(0.5f, 1.f);
						std::uniform_real_distribution<float> velocityDis(3.f, 5.f);
						for (unsigned int i = startId; i <= endId; ++i)
						{
							Nz::Vector3f normal(normalDis(m_randomGenerator), normalDis(m_randomGenerator), normalDis(m_randomGenerator));
							normal.Normalize();

							particleColor[i] = Nz::Color::White;
							particleLife[i] = lifeDis(m_randomGenerator);
							particleRotation[i] = rotDis(m_randomGenerator);
							particleSize[i] = Nz::Vector2f(sizeDis(m_randomGenerator));
							particleVel[i] = normal * velocityDis(m_randomGenerator);
						}
					}));

					Nz::MaterialRef fireMaterial = Nz::Material::New("Translucent3D");
					fireMaterial->SetDiffuseMap(assetsFolder + "particles/fire_particle.png");

					entityGroup.SetRenderer(Nz::ParticleFunctionRenderer::New([fireMaterial](const Nz::ParticleGroup& /*group*/, const Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId, Nz::AbstractRenderQueue* renderQueue)
					{
						auto particlePos = mapper.GetComponentPtr<Nz::Vector3f>(Nz::ParticleComponent_Position) + startId;
						auto particleColor = mapper.GetComponentPtr<Nz::Color>(Nz::ParticleComponent_Color) + startId;
						auto particleRotation = mapper.GetComponentPtr<float>(Nz::ParticleComponent_Rotation) + startId;
						auto particleSize = mapper.GetComponentPtr<Nz::Vector2f>(Nz::ParticleComponent_Size) + startId;

						renderQueue->AddBillboards(0, fireMaterial, endId - startId + 1, Nz::Recti(-1, -1), particlePos, particleSize, particleRotation, particleColor);
					}));

					particleGroup.instantiate = [this](const Ndk::EntityHandle& group, const Nz::Vector3f& position, const Nz::Quaternionf& /*rotation*/)
					{
						std::uniform_int_distribution<unsigned int> particleCountDis(200, 300);
						std::uniform_real_distribution<float> posDis(-0.5f, 0.5f);

						unsigned int particleCount = particleCountDis(m_randomGenerator);

						auto& entityGroup = group->GetComponent<Ndk::ParticleGroupComponent>();
						Nz::ParticleStruct_Billboard* particles = static_cast<Nz::ParticleStruct_Billboard*>(entityGroup.GenerateParticles(particleCount));
						if (!particles)
							return;

						for (unsigned int i = 0; i < particleCount; ++i)
							particles[i].position = position + Nz::Vector3f(posDis(m_randomGenerator), posDis(m_randomGenerator), posDis(m_randomGenerator));
					};
				}
				else if (particleGroupName == "explosion_smoke")
				{
					auto& entityGroup = particleGroup.particleGroup->AddComponent<Ndk::ParticleGroupComponent>(10'000, Nz::ParticleLayout_Billboard);

					//entityGroup.AddController(std::make_unique<AlphaController>(50.f).release());
					entityGroup.AddController(std::make_unique<LifeController>().release());
					entityGroup.AddController(std::make_unique<GrowthController>(2.f).release());
					entityGroup.AddController(std::make_unique<VelocityController>().release());

					static constexpr float maxSmokeLife = 7.f;
					entityGroup.AddController(Nz::ParticleFunctionController::New([this](Nz::ParticleGroup& group, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId, float elapsedTime)
					{
						auto colorPtr = mapper.GetComponentPtr<Nz::Color>(Nz::ParticleComponent_Color);
						auto lifePtr = mapper.GetComponentPtr<float>(Nz::ParticleComponent_Life);

						for (unsigned int i = startId; i <= endId; ++i)
						{
							float alpha = std::min((maxSmokeLife - lifePtr[i]) * 255.f / 5.f, 255.f);
							alpha -= std::max((maxSmokeLife - lifePtr[i]) / maxSmokeLife * 255.f, 0.f);

							colorPtr[i].a = static_cast<Nz::UInt8>(Nz::Clamp(alpha, 0.f, 255.f));
						}
					}));

					entityGroup.AddGenerator(Nz::ParticleFunctionGenerator::New([this](Nz::ParticleGroup& /*group*/, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId)
					{
						auto particleColor = mapper.GetComponentPtr<Nz::Color>(Nz::ParticleComponent_Color);
						auto particleRotation = mapper.GetComponentPtr<float>(Nz::ParticleComponent_Rotation);
						auto particleLife = mapper.GetComponentPtr<float>(Nz::ParticleComponent_Life);
						auto particleSize = mapper.GetComponentPtr<Nz::Vector2f>(Nz::ParticleComponent_Size);
						auto particleVel = mapper.GetComponentPtr<Nz::Vector3f>(Nz::ParticleComponent_Velocity);

						std::uniform_real_distribution<float> normalDis(-1.f, 1.f);
						std::uniform_real_distribution<float> rotDis(-180.f, 180.f);
						std::uniform_real_distribution<float> lifeDis(5.f, maxSmokeLife);
						std::uniform_real_distribution<float> sizeDis(3.f, 5.f);
						std::uniform_real_distribution<float> velocityDis(0.5f, 1.f);
						for (unsigned int i = startId; i <= endId; ++i)
						{
							Nz::Vector3f normal(normalDis(m_randomGenerator), normalDis(m_randomGenerator), normalDis(m_randomGenerator));
							normal.Normalize();

							particleColor[i] = Nz::Color::White;
							particleColor[i].a = 0;
							particleLife[i] = lifeDis(m_randomGenerator);
							particleRotation[i] = rotDis(m_randomGenerator);
							particleSize[i] = Nz::Vector2f(sizeDis(m_randomGenerator));
							particleVel[i] = normal * velocityDis(m_randomGenerator);
						}
					}));

					Nz::MaterialRef smokeMaterial = Nz::Material::New("Translucent3D");
					smokeMaterial->SetDiffuseMap(assetsFolder + "particles/smoke.png");

					entityGroup.SetRenderer(Nz::ParticleFunctionRenderer::New([smokeMaterial](const Nz::ParticleGroup& /*group*/, const Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId, Nz::AbstractRenderQueue* renderQueue)
					{
						auto particlePos = mapper.GetComponentPtr<Nz::Vector3f>(Nz::ParticleComponent_Position) + startId;
						auto particleColor = mapper.GetComponentPtr<Nz::Color>(Nz::ParticleComponent_Color) + startId;
						auto particleRotation = mapper.GetComponentPtr<float>(Nz::ParticleComponent_Rotation) + startId;
						auto particleSize = mapper.GetComponentPtr<Nz::Vector2f>(Nz::ParticleComponent_Size) + startId;

						renderQueue->AddBillboards(0, smokeMaterial, endId - startId + 1, Nz::Recti(-1, -1), particlePos, particleSize, particleRotation, particleColor);
					}));

					particleGroup.instantiate = [this](const Ndk::EntityHandle& group, const Nz::Vector3f& position, const Nz::Quaternionf& /*rotation*/)
					{
						std::uniform_int_distribution<unsigned int> particleCountDis(10, 20);
						std::uniform_real_distribution<float> posDis(-0.2f, 0.2f);

						unsigned int particleCount = particleCountDis(m_randomGenerator);

						auto& entityGroup = group->GetComponent<Ndk::ParticleGroupComponent>();
						Nz::ParticleStruct_Billboard* particles = static_cast<Nz::ParticleStruct_Billboard*>(entityGroup.GenerateParticles(particleCount));
						if (!particles)
							return;

						for (unsigned int i = 0; i < particleCount; ++i)
							particles[i].position = position + Nz::Vector3f(posDis(m_randomGenerator), posDis(m_randomGenerator), posDis(m_randomGenerator));
					};
				}
				else if (particleGroupName == "explosion_wave")
				{
					particleGroup.instantiate = [this](const Ndk::EntityHandle& group, const Nz::Vector3f& position, const Nz::Quaternionf& /*rotation*/)
					{
					};
				}
			}
		}
	}

	void ServerMatchEntities::OnArenaSounds(ServerConnection* server, const Packets::ArenaSounds& arenaSounds)
	{
		Nz::SoundBufferParams fileParams;
		fileParams.forceMono = true;

		const std::string& assetsFolder = server->GetApp().GetConfig().GetStringOption("AssetsFolder");

		m_soundLibrary.erase(m_soundLibrary.begin() + arenaSounds.startId, m_soundLibrary.end());
		for (const auto& sound : arenaSounds.sounds)
		{
			std::string filePath = assetsFolder + '/' + sound.filePath;

			Nz::SoundBufferRef soundBuffer = Nz::SoundBuffer::LoadFromFile(filePath, fileParams);
			if (!soundBuffer)
				std::cerr << "Failed to load " << filePath << std::endl;

			m_soundLibrary.emplace_back(std::move(soundBuffer));
		}
	}

	void ServerMatchEntities::OnArenaState(ServerConnection* server, const Packets::ArenaState& arenaState)
	{
		// For now, allocate a new snapshot, we will recycle them in a further iteration (to prevent memory allocation)
		Snapshot snapshot;
		snapshot.entities.resize(arenaState.entities.size());
		for (std::size_t i = 0; i < snapshot.entities.size(); ++i)
		{
			Snapshot::Entity& entity = snapshot.entities[i];
			const Packets::ArenaState::Entity& packetEntity = arenaState.entities[i];

			entity.id = packetEntity.id;
			entity.angularVelocity = packetEntity.angularVelocity;
			entity.linearVelocity = packetEntity.linearVelocity;
			entity.position = packetEntity.position;
			entity.rotation = packetEntity.rotation;
		}

		snapshot.applyTime = arenaState.serverTime + m_snapshotDelay;
		snapshot.stateId = arenaState.stateId;

		m_jitterBuffer.push_back(std::move(snapshot));
	}

	void ServerMatchEntities::OnCreateEntities(ServerConnection*, const Packets::CreateEntities& createPacket)
	{
		for (const auto& entityData : createPacket.entities)
		{
			ServerEntity& data = CreateServerEntity(entityData.entityId);

			data.positionError = Nz::Vector3f::Zero();
			data.rotationError = Nz::Quaternionf::Identity();

			data.entity = m_prefabs[entityData.prefabId]->Clone();

			data.name = entityData.visualName.ToStdString();

			auto& entityNode = data.entity->GetComponent<Ndk::NodeComponent>();
			entityNode.SetPosition(entityData.position);
			entityNode.SetRotation(entityData.rotation);

			auto& entityPhys = data.entity->GetComponent<Ndk::PhysicsComponent3D>();
			entityPhys.SetAngularVelocity(entityData.angularVelocity);
			entityPhys.SetLinearVelocity(entityData.linearVelocity);
			entityPhys.SetPosition(entityData.position);
			entityPhys.SetRotation(entityData.rotation);

			if (data.entity->HasComponent<SoundEmitterComponent>())
			{
				auto& soundEmitter = data.entity->GetComponent<SoundEmitterComponent>();
				soundEmitter.Play();
			}

			Nz::Color textColor = (entityData.visualName == "Lynix") ? Nz::Color::Cyan : Nz::Color::White;

			// Create entity name entity
			if (!entityData.visualName.IsEmpty())
			{
				Nz::TextSpriteRef textSprite = Nz::TextSprite::New();
				textSprite->SetMaterial(Nz::MaterialLibrary::Get("SpaceshipText"));
				textSprite->Update(Nz::SimpleTextDrawer::Draw(entityData.visualName, 96, 0U, textColor));
				textSprite->SetScale(0.01f);

				data.textEntity = m_world->CreateEntity();
				data.textEntity->AddComponent<Ndk::GraphicsComponent>().Attach(textSprite);
				data.textEntity->AddComponent<Ndk::NodeComponent>();
			}

			OnEntityCreated(this, data);
		}
	}

	void ServerMatchEntities::OnDeleteEntities(ServerConnection*, const Packets::DeleteEntities& deletePacket)
	{
		for (std::size_t entityId : deletePacket.entities)
		{
			ServerEntity& data = GetServerEntity(entityId);

			if (data.debugGhostEntity)
				data.debugGhostEntity->Kill();

			if (data.textEntity)
				data.textEntity->Kill();

			data.entity->Kill();
			data.isValid = false;

			OnEntityDelete(this, data);
		}
	}

	void ServerMatchEntities::OnInstantiateParticleSystem(ServerConnection* server, const Packets::InstantiateParticleSystem& instantiatePacket)
	{
		ParticleSystem& particleSystem = m_particleSystems[instantiatePacket.particleSystemId];
		for (const auto& particleGroup : particleSystem.particleGroups)
		{
			particleGroup.instantiate(particleGroup.particleGroup, instantiatePacket.position, instantiatePacket.rotation);
		}
	}

	void ServerMatchEntities::OnPlaySound(ServerConnection* server, const Packets::PlaySound& playSound)
	{
		Nz::Sound& sound = m_playingSounds.emplace_back();
		sound.SetBuffer(m_soundLibrary[playSound.soundId]);
		sound.EnableSpatialization(true);
		sound.SetPosition(playSound.position);
		sound.SetMinDistance(50.f);

		sound.Play();
	}

	void ServerMatchEntities::ApplySnapshot(const Snapshot& snapshot)
	{
		//std::cout << "Applied snapshot #" << snapshot.stateId << " after " << (m_server->EstimateServerTime() - snapshot.applyTime) << "ms" << std::endl;
		for (const Snapshot::Entity& entityData : snapshot.entities)
		{
			if (!IsServerEntityValid(entityData.id))
				continue;

			ServerEntity& data = GetServerEntity(entityData.id);

			auto& entityNode = data.entity->GetComponent<Ndk::NodeComponent>();
			auto& entityPhys = data.entity->GetComponent<Ndk::PhysicsComponent3D>();

			// Compute visual error
			Nz::Quaternionf visualRotation = entityNode.GetRotation();
			Nz::Vector3f visualPosition = entityNode.GetPosition();

			/*data.positionError = Nz::Vector3f::Zero();
			data.rotationError = Nz::Quaternionf::Identity();*/

			data.positionError += entityPhys.GetPosition() - entityData.position;
			data.rotationError = data.rotationError * entityData.rotation.GetConjugate() * entityPhys.GetRotation();

			/*if (float diff = Nz::Vector3f::Distance(entityPhys.GetPosition() + data.positionError, visualPosition); diff > 0.0001f)
				std::cout << "Distance: " << Nz::Vector3f::Distance(entityPhys.GetPosition() + data.positionError, visualPosition) << std::endl;

			Nz::Quaternionf test = visualRotation * (entityPhys.GetRotation() * data.rotationError).GetInverse();
			Nz::Vector3f testVec(test.x, test.y, test.z);

			if (float diff = std::atan2(testVec.GetLength(), test.w); diff > 0.0001f)
				std::cout << "Rotation: " << diff << std::endl;*/

			// Hard apply physics state
			entityPhys.SetAngularVelocity(entityData.angularVelocity);
			entityPhys.SetLinearVelocity(entityData.linearVelocity);
			entityPhys.SetPosition(entityData.position);
			entityPhys.SetRotation(entityData.rotation);
		}
	}
}
