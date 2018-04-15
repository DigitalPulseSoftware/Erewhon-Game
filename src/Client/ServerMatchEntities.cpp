// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ServerMatchEntities.hpp>
#include <Nazara/Core/Clock.hpp>
#include <Nazara/Core/Primitive.hpp>
#include <Nazara/Graphics/Billboard.hpp>
#include <Nazara/Graphics/Model.hpp>
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

		m_onArenaPrefabsSlot.Connect(server->OnArenaPrefabs, this, &ServerMatchEntities::OnArenaPrefabs);
		m_onArenaSoundsSlot.Connect(server->OnArenaSounds, this,   &ServerMatchEntities::OnArenaSounds);
		m_onArenaStateSlot.Connect(server->OnArenaState, this,     &ServerMatchEntities::OnArenaState);
		m_onCreateEntitySlot.Connect(server->OnCreateEntity, this, &ServerMatchEntities::OnCreateEntity);
		m_onDeleteEntitySlot.Connect(server->OnDeleteEntity, this, &ServerMatchEntities::OnDeleteEntity);
		m_onPlaySoundSlot.Connect(server->OnPlaySound, this,       &ServerMatchEntities::OnPlaySound);

		FillVisualEffectFactory();

		// Listen to debug state
		if constexpr (showServerGhosts)
		{
			m_debugStateSocket.Create(Nz::NetProtocol_IPv4);
			m_debugStateSocket.Bind(2050);

			m_debugStateSocket.EnableBlocking(false);
		}
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
		m_visualEffectFactory["earth"] = [](ClientApplication* /*app*/, const Ndk::EntityHandle& entity)
		{
			Nz::MeshRef earthMesh = Nz::Mesh::New();
			earthMesh->CreateStatic();
			earthMesh->BuildSubMesh(Nz::Primitive::UVSphere(50.f, 40, 40));

			Nz::MaterialRef earthMaterial = Nz::Material::New();
			earthMaterial->SetDiffuseMap("Assets/earth/earthmap1k.jpg");
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
		Nz::ModelParameters params;
		params.mesh.center = true;
		params.mesh.texCoordScale.Set(1.f, -1.f);

		m_prefabs.erase(m_prefabs.begin() + arenaPrefabs.startId, m_prefabs.end());

		const std::string& assetsFolder = server->GetApp().GetConfig().GetStringOption("AssetsFolder");

		const NetworkStringStore& networkStringStore = server->GetNetworkStringStore();
		for (const auto& prefab : arenaPrefabs.prefabs)
		{
			const Ndk::EntityHandle& entity = m_world->CreateEntity();
			entity->Disable();

			entity->AddComponent<Ndk::NodeComponent>();

			auto& physComponent = entity->AddComponent<Ndk::PhysicsComponent3D>();
			physComponent.EnableNodeSynchronization(false);
			physComponent.SetMass(10.f);
			physComponent.SetAngularDamping(Nz::Vector3f(0.f));
			physComponent.SetLinearDamping(0.f);

			auto& graphicsComponent = entity->AddComponent<Ndk::GraphicsComponent>();
			for (const auto& modelPiece : prefab.models)
			{
				Nz::Matrix4f transformMatrix = Nz::Matrix4f::Transform(modelPiece.position, modelPiece.rotation, modelPiece.scale);

				// TODO: Load it once for every path
				std::string filePath = assetsFolder + '/' + networkStringStore.GetString(modelPiece.modelId);

				Nz::ModelRef model = Nz::Model::New();
				if (model->LoadFromFile(filePath, params))
					graphicsComponent.Attach(model, transformMatrix);
				else
					std::cerr << "Failed to load " << filePath << std::endl;
			}

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

	void ServerMatchEntities::OnArenaSounds(ServerConnection* server, const Packets::ArenaSounds& arenaSounds)
	{
		Nz::SoundBufferParams fileParams;
		fileParams.forceMono = true;

		const std::string& assetsFolder = server->GetApp().GetConfig().GetStringOption("AssetsFolder");

		m_soundLibrary.erase(m_soundLibrary.begin() + arenaSounds.startId, m_soundLibrary.end());
		for (const auto& sound : arenaSounds.sounds)
		{
			std::string filePath = assetsFolder + '/' + sound.filePath;

			Nz::SoundBufferRef soundBuffer = Nz::SoundBuffer::New();
			if (!soundBuffer->LoadFromFile(filePath, fileParams))
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

	void ServerMatchEntities::OnCreateEntity(ServerConnection*, const Packets::CreateEntity& createPacket)
	{
		ServerEntity& data = CreateServerEntity(createPacket.entityId);

		data.positionError = Nz::Vector3f::Zero();
		data.rotationError = Nz::Quaternionf::Identity();

		data.entity = m_prefabs[createPacket.prefabId]->Clone();

		data.name = createPacket.visualName.ToStdString();

		auto& entityNode = data.entity->GetComponent<Ndk::NodeComponent>();
		entityNode.SetPosition(createPacket.position);
		entityNode.SetRotation(createPacket.rotation);

		auto& entityPhys = data.entity->GetComponent<Ndk::PhysicsComponent3D>();
		entityPhys.SetAngularVelocity(createPacket.angularVelocity);
		entityPhys.SetLinearVelocity(createPacket.linearVelocity);
		entityPhys.SetPosition(createPacket.position);
		entityPhys.SetRotation(createPacket.rotation);

		if (data.entity->HasComponent<SoundEmitterComponent>())
		{
			auto& soundEmitter = data.entity->GetComponent<SoundEmitterComponent>();
			soundEmitter.Play();
		}

		Nz::Color textColor = (createPacket.visualName == "Lynix") ? Nz::Color::Cyan : Nz::Color::White;

		// Create entity name entity
		if (!createPacket.visualName.IsEmpty())
		{
			Nz::TextSpriteRef textSprite = Nz::TextSprite::New();
			textSprite->SetMaterial(Nz::MaterialLibrary::Get("SpaceshipText"));
			textSprite->Update(Nz::SimpleTextDrawer::Draw(createPacket.visualName, 96, 0U, textColor));
			textSprite->SetScale(0.01f);

			data.textEntity = m_world->CreateEntity();
			data.textEntity->AddComponent<Ndk::GraphicsComponent>().Attach(textSprite);
			data.textEntity->AddComponent<Ndk::NodeComponent>();
		}

		OnEntityCreated(this, data);
	}

	void ServerMatchEntities::OnDeleteEntity(ServerConnection*, const Packets::DeleteEntity& deletePacket)
	{
		ServerEntity& data = GetServerEntity(deletePacket.id);

		if (data.debugGhostEntity)
			data.debugGhostEntity->Kill();

		if (data.textEntity)
			data.textEntity->Kill();

		data.entity->Kill();
		data.isValid = false;

		OnEntityDelete(this, data);
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
