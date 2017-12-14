// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ServerMatchEntities.hpp>
#include <Nazara/Core/Clock.hpp>
#include <Nazara/Core/Primitive.hpp>
#include <Nazara/Graphics/Model.hpp>
#include <Nazara/Graphics/TextSprite.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/Components.hpp>
#include <Client/ClientApplication.hpp>
#include <iostream>

namespace ewn
{
	static constexpr bool showServerGhosts = true;

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

		m_onArenaStateSlot.Connect(server->OnArenaState, this, &ServerMatchEntities::OnArenaState);
		m_onCreateEntitySlot.Connect(server->OnCreateEntity, this, &ServerMatchEntities::OnCreateEntity);
		m_onDeleteEntitySlot.Connect(server->OnDeleteEntity, this, &ServerMatchEntities::OnDeleteEntity);

		CreateEntityTemplates();

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

				spaceshipData.positionError = Nz::Lerp(spaceshipData.positionError, Nz::Vector3f::Zero(), 0.1f);

				// Avoid denormals
				if (Nz::NumberEquals(spaceshipData.positionError.x, 0.f, 0.001f) &&
					Nz::NumberEquals(spaceshipData.positionError.y, 0.f, 0.001f) &&
					Nz::NumberEquals(spaceshipData.positionError.z, 0.f, 0.001f))
				{
					spaceshipData.positionError = Nz::Vector3f::Zero();
				}

				spaceshipData.rotationError = Nz::Quaternionf::Slerp(spaceshipData.rotationError, Nz::Quaternionf::Identity(), 0.1f);

				//if (spaceshipData.entity->GetId() == 9)
				//	std::cout << "#" << spaceshipData.entity->GetId() << ": " << entityPhys.GetLinearVelocity() << " " << entityPhys.GetAngularVelocity() << std::endl;

				entityNode.SetPosition(entityPhys.GetPosition() + spaceshipData.positionError);
				entityNode.SetRotation(entityPhys.GetRotation() * spaceshipData.rotationError);
			}
		}

		if constexpr (showServerGhosts)
		{
			Nz::NetPacket packet;
			if (m_debugStateSocket.ReceivePacket(&packet, nullptr))
			{
				Packets::ArenaState arenaState;
				Packets::Unserialize(packet, arenaState);

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
		}
	}

	void ServerMatchEntities::CreateEntityTemplates()
	{
		Nz::ModelParameters params;
		params.mesh.center = true;
		params.material.shaderName = "Basic";

		// Ball
		Nz::ModelRef ballModel = Nz::Model::New();
		ballModel->LoadFromFile("Assets/ball/ball.obj", params);

		{
			m_ballTemplateEntity = m_world->CreateEntity();

			constexpr float radius = 18.251904f / 2.f;

			m_ballTemplateEntity->AddComponent<Ndk::CollisionComponent3D>(Nz::SphereCollider3D::New(radius));
			m_ballTemplateEntity->AddComponent<Ndk::GraphicsComponent>().Attach(ballModel);
			m_ballTemplateEntity->AddComponent<Ndk::NodeComponent>();

			auto& physComponent = m_ballTemplateEntity->AddComponent<Ndk::PhysicsComponent3D>();
			physComponent.EnableNodeSynchronization(false);
			physComponent.SetMass(10.f);
			physComponent.SetAngularVelocity(Nz::Vector3f(0.f));
			physComponent.SetLinearDamping(0.f);

			m_ballTemplateEntity->Disable();
		}

		// Earth
		{
			Nz::MeshRef earthMesh = Nz::Mesh::New();
			earthMesh->CreateStatic();
			earthMesh->BuildSubMesh(Nz::Primitive::UVSphere(1.f, 40, 40));

			Nz::MaterialRef earthMaterial = Nz::Material::New();
			earthMaterial->SetDiffuseMap("Assets/earth/earthmap1k.jpg");
			earthMaterial->SetShader("Basic");

			Nz::ModelRef earthModel = Nz::Model::New();
			earthModel->SetMesh(earthMesh);
			earthModel->SetMaterial(0, earthMaterial);

			m_earthTemplateEntity = m_world->CreateEntity();
			m_earthTemplateEntity->AddComponent<Ndk::CollisionComponent3D>(Nz::SphereCollider3D::New(20.f));
			m_earthTemplateEntity->AddComponent<Ndk::GraphicsComponent>().Attach(earthModel);

			auto& physComponent = m_earthTemplateEntity->AddComponent<Ndk::PhysicsComponent3D>();
			physComponent.EnableNodeSynchronization(false);
			physComponent.SetMass(0.f);
			physComponent.SetAngularVelocity(Nz::Vector3f(0.f));
			physComponent.SetLinearDamping(0.f);

			auto& earthNode = m_earthTemplateEntity->AddComponent<Ndk::NodeComponent>();
			earthNode.SetPosition(Nz::Vector3f::Forward() * 50.f);
			earthNode.SetRotation(Nz::EulerAnglesf(0.f, 180.f, 0.f));
			earthNode.SetScale(20.f);

			m_earthTemplateEntity->Disable();
		}

		// Spaceship
		params.mesh.matrix.MakeTransform(Nz::Vector3f::Zero(), Nz::EulerAnglesf(0.f, 90.f, 0.f), Nz::Vector3f(0.01f));
		params.mesh.texCoordScale.Set(1.f, -1.f);

		Nz::ModelRef spaceshipModel = Nz::Model::New();
		spaceshipModel->LoadFromFile("Assets/spaceship/spaceship.obj", params);

		{
			m_spaceshipTemplateEntity = m_world->CreateEntity();

			Nz::SphereCollider3DRef collider = Nz::SphereCollider3D::New(5.f);
			//auto& collisionComponent = m_spaceshipTemplateEntity->AddComponent<Ndk::CollisionComponent3D>(collider);

			m_spaceshipTemplateEntity->AddComponent<Ndk::GraphicsComponent>().Attach(spaceshipModel);
			m_spaceshipTemplateEntity->AddComponent<Ndk::NodeComponent>();
			auto& spaceshipPhys = m_spaceshipTemplateEntity->AddComponent<Ndk::PhysicsComponent3D>();
			spaceshipPhys.EnableNodeSynchronization(false);
			spaceshipPhys.SetMass(42.f);
			spaceshipPhys.SetAngularDamping(Nz::Vector3f(0.f));
			spaceshipPhys.SetLinearDamping(0.f);

			m_spaceshipTemplateEntity->Disable();
		}

		Nz::MaterialRef debugMaterial = Nz::Material::New("Translucent3D");
		debugMaterial->SetDiffuseColor(Nz::Color(255, 255, 255, 50));

		Nz::ModelRef ghostSpaceship = Nz::Model::New(*spaceshipModel);
		for (std::size_t i = 0; i < ghostSpaceship->GetMaterialCount(); ++i)
			ghostSpaceship->SetMaterial(i, debugMaterial);

		m_debugTemplateEntity = m_world->CreateEntity();
		m_debugTemplateEntity->AddComponent<Ndk::GraphicsComponent>().Attach(ghostSpaceship);
		m_debugTemplateEntity->AddComponent<Ndk::NodeComponent>();
		m_debugTemplateEntity->Disable();

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
		ServerEntity& data = CreateServerEntity(createPacket.id);

		data.positionError = Nz::Vector3f::Zero();
		data.rotationError = Nz::Quaternionf::Identity();

		if (createPacket.entityType == "spaceship")
			data.entity = m_spaceshipTemplateEntity->Clone();
		else if (createPacket.entityType == "earth")
			data.entity = m_earthTemplateEntity->Clone();
		else if (createPacket.entityType == "ball")
			data.entity = m_ballTemplateEntity->Clone();
		else if (createPacket.entityType == "projectile")
		{
			data.entity = m_ballTemplateEntity->Clone();
			data.entity->GetComponent<Ndk::NodeComponent>().SetScale(1.f / 5.f);
		}
		else
			return; //< TODO: Fallback

		auto& entityNode = data.entity->GetComponent<Ndk::NodeComponent>();
		entityNode.SetPosition(createPacket.position);
		entityNode.SetRotation(createPacket.rotation);

		Nz::Color textColor = (createPacket.name == "Lynix") ? Nz::Color::Cyan : Nz::Color::White;

		// Create entity name entity
		Nz::TextSpriteRef textSprite = Nz::TextSprite::New();
		textSprite->SetMaterial(Nz::MaterialLibrary::Get("SpaceshipText"));
		textSprite->Update(Nz::SimpleTextDrawer::Draw(createPacket.name, 96, 0U, textColor));
		textSprite->SetScale(0.01f);

		data.textEntity = m_world->CreateEntity();
		data.textEntity->AddComponent<Ndk::GraphicsComponent>().Attach(textSprite);
		data.textEntity->AddComponent<Ndk::NodeComponent>();

		OnEntityCreated(this, data);
	}

	void ServerMatchEntities::OnDeleteEntity(ServerConnection*, const Packets::DeleteEntity& deletePacket)
	{
		ServerEntity& data = GetServerEntity(deletePacket.id);

		if (data.debugGhostEntity)
			data.debugGhostEntity->Kill();

		data.entity->Kill();
		data.textEntity->Kill();
		data.isValid = false;

		OnEntityDelete(this, data);
	}

	void ServerMatchEntities::ApplySnapshot(const Snapshot& snapshot)
	{
		std::cout << "Applied snapshot #" << snapshot.stateId << " after " << (m_server->EstimateServerTime() - snapshot.applyTime) << "ms" << std::endl;
		for (const Snapshot::Entity& entityData : snapshot.entities)
		{
			ServerEntity& data = GetServerEntity(entityData.id);

			auto& entityNode = data.entity->GetComponent<Ndk::NodeComponent>();
			auto& entityPhys = data.entity->GetComponent<Ndk::PhysicsComponent3D>();

			// Hard apply physics state
			entityPhys.SetAngularVelocity(entityData.angularVelocity);
			entityPhys.SetLinearVelocity(entityData.linearVelocity);
			entityPhys.SetPosition(entityData.position);
			entityPhys.SetRotation(entityData.rotation);

			// Compute visual error
			Nz::Quaternionf visualRotation = entityNode.GetRotation();
			Nz::Vector3f visualPosition = entityNode.GetPosition();

			//data.positionError = Nz::Vector3f::Zero();
			//data.rotationError = Nz::Quaternionf::Identity();

			data.positionError = visualPosition - entityData.position;
			data.rotationError = entityData.rotation.GetConjugate() * visualRotation;

			Nz::Quaternionf test = visualRotation * (entityPhys.GetRotation() * data.rotationError).GetInverse();
			Nz::Vector3f testVec(test.x, test.y, test.z);

			if (float diff = Nz::Vector3f::Distance(entityPhys.GetPosition() + data.positionError, visualPosition); diff > 0.0001f)
				std::cout << "Distance: " << Nz::Vector3f::Distance(entityPhys.GetPosition() + data.positionError, visualPosition) << std::endl;

			if (float diff = std::atan2(testVec.GetLength(), test.w); diff > 0.001f)
				std::cout << "Rotation: " << diff << std::endl;
		}
	}
}
