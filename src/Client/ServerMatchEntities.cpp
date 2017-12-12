// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ServerMatchEntities.hpp>
#include <Nazara/Core/Primitive.hpp>
#include <Nazara/Graphics/Model.hpp>
#include <Nazara/Graphics/TextSprite.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/Components.hpp>
#include <iostream>

namespace ewn
{
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
		constexpr float snapshotUpdateInterval = 1.f / 9.f;

		m_snapshotUpdateAccumulator += elapsedTime;
		if (m_snapshotUpdateAccumulator > snapshotUpdateInterval)
		{
			// Don't treat this timer like others: reset accumulator everytime to prevent multiple ticks applications
			// This will allow the jitter cursor to stay low (because client will update snapshots at a slightly lower rate)
			// and physics correction system will handle deviations caused by this
			m_snapshotUpdateAccumulator = 0;

			HandleNextSnapshot();
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

				spaceshipData.positionError = Nz::Lerp(spaceshipData.positionError, Nz::Vector3f::Zero(), 0.01f);

				// Avoid denormals
				if (Nz::NumberEquals(spaceshipData.positionError.x, 0.f, 0.001f) &&
					Nz::NumberEquals(spaceshipData.positionError.y, 0.f, 0.001f) &&
					Nz::NumberEquals(spaceshipData.positionError.z, 0.f, 0.001f))
				{
					spaceshipData.positionError = Nz::Vector3f::Zero();
				}

				spaceshipData.rotationError = Nz::Quaternionf::Slerp(spaceshipData.rotationError, Nz::Quaternionf::Identity(), 0.1f);

				if (spaceshipData.entity->GetId() == 9)
					std::cout << "#" << spaceshipData.entity->GetId() << ": " << spaceshipData.positionError << " " << spaceshipData.rotationError << std::endl;

				entityNode.SetPosition(entityPhys.GetPosition() + spaceshipData.positionError);
				entityNode.SetRotation(entityPhys.GetRotation() * spaceshipData.rotationError);
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
			m_earthTemplateEntity->AddComponent<Ndk::PhysicsComponent3D>().EnableNodeSynchronization(false);

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
			auto& collisionComponent = m_spaceshipTemplateEntity->AddComponent<Ndk::CollisionComponent3D>(collider);

			m_spaceshipTemplateEntity->AddComponent<Ndk::GraphicsComponent>().Attach(spaceshipModel);
			m_spaceshipTemplateEntity->AddComponent<Ndk::NodeComponent>();
			auto& spaceshipPhys = m_spaceshipTemplateEntity->AddComponent<Ndk::PhysicsComponent3D>();
			spaceshipPhys.EnableNodeSynchronization(false);
			spaceshipPhys.SetMass(42.f);
			spaceshipPhys.SetAngularDamping(Nz::Vector3f(0.3f));
			spaceshipPhys.SetLinearDamping(0.25f);

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
		std::cout << "Received " << arenaState.stateId << " at " << server->EstimateServerTime() << std::endl;

		if (m_jitterBufferSize >= m_jitterBuffer.size())
		{
			// Jitter buffer cannot grow anymore, drop states
			Nz::UInt16 expectedStateId = m_jitterBuffer[m_jitterBufferSize - 1].stateId + 1;
			std::size_t missedStates = arenaState.stateId - expectedStateId;
			if (missedStates > 0)
				std::cout << "Missed " << missedStates << " states!" << std::endl;

			if (missedStates < m_jitterBuffer.size() - 1)
			{
				std::rotate(m_jitterBuffer.begin(), m_jitterBuffer.begin() + missedStates + 1, m_jitterBuffer.begin() + m_jitterBufferSize);

				CopyState(m_jitterBuffer.size() - 1, arenaState);

				MarkStateAsLost(m_jitterBuffer.size() - missedStates - 1, m_jitterBuffer.size() - 1);
			}
			else
				// Lost too many states, reset jitter buffer
				ResetSnapshots(arenaState);
		}
		else if (m_jitterBufferSize > 0)
		{
			// Grow jitter buffer

			Nz::UInt16 expectedStateId = m_jitterBuffer[m_jitterBufferSize - 1].stateId + 1;
			std::size_t newStates = arenaState.stateId - expectedStateId + 1;
			if (newStates > 1)
				std::cout << "Missed " << newStates - 1 << " states!" << std::endl;

			if (newStates < m_jitterBuffer.size())
			{
				std::size_t spaceLeft = m_jitterBuffer.size() - m_jitterBufferSize;
				assert(spaceLeft > 0);

				if (spaceLeft < newStates)
				{
					std::size_t difference = newStates - spaceLeft;
					std::rotate(m_jitterBuffer.begin(), m_jitterBuffer.begin() + difference, m_jitterBuffer.begin() + m_jitterBufferSize);
					m_jitterBufferSize -= difference;
				}

				std::size_t newSize = m_jitterBufferSize + newStates;

				MarkStateAsLost(m_jitterBufferSize, newSize - 1);
				CopyState(newSize - 1, arenaState);

				m_jitterBufferSize = newSize;
			}
			else
				ResetSnapshots(arenaState);
		}
		else
		{
			// Jitter buffer is empty, just insert state
			CopyState(m_jitterBufferSize, arenaState);
			m_jitterBufferSize++;
		}


		/*Nz::UInt16 expectedStateId = m_jitterBuffer[m_jitterBufferSize - 1].stateId + 1;
		if (arenaState.stateId >= expectedStateId)
		{
			std::size_t missedStates = arenaState.stateId - expectedStateId;
			if (missedStates > 0)
				std::cout << "Missed " << missedStates << " states!" << std::endl;

			if (missedStates < m_jitterBuffer.size() - 1)
			{
				std::rotate(m_jitterBuffer.begin(), m_jitterBuffer.begin() + missedStates + 1, m_jitterBuffer.end());

				m_jitterBuffer.back().stateId = arenaState.stateId;

				// Recreate missed states
				MarkStateAsLost(m_jitterBuffer.size() - missedStates - 2, m_jitterBuffer.size() - 1);

				if (m_jitterCursorBegin > missedStates)
					m_jitterCursorBegin -= missedStates + 1;
				else
					m_jitterCursorBegin = 0;
			}
			else
				ResetSnapshots(arenaState);
		}
		else
		{
			std::cout << "Out-of-order packet!" << std::endl;
			return; // For now, drop out-of-order packets
		}*/

		bool valid = true;
		std::size_t firstValidId = m_jitterBufferSize;
		for (Nz::UInt16 i = 0; i < m_jitterBufferSize; ++i)
		{
			const auto& snapshot = m_jitterBuffer[i];
			if (!snapshot.isValid)
				continue;

			if (firstValidId == m_jitterBufferSize)
				firstValidId = i;

			if (snapshot.stateId != m_jitterBuffer[firstValidId].stateId + (i - firstValidId))
			{
				valid = false;
				break;
			}
		}

		if (!valid)
		{
			std::cout << "Invalid jitter buffer: [";
			for (Nz::UInt16 i = 0; i < m_jitterBufferSize; ++i)
			{
				const auto& snapshot = m_jitterBuffer[i];
				if (snapshot.isValid)
					std::cout << snapshot.stateId << ' ';
				else
					std::cout << "Lost ";
			}

			std::cout << ']' << std::endl;
		}
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

			data.positionError = visualPosition - entityData.position;
			data.rotationError = entityData.rotation.GetConjugate() * visualRotation;

			Nz::Quaternionf test = visualRotation * (entityPhys.GetRotation() * data.rotationError).GetInverse();
			Nz::Vector3f testVec(test.x, test.y, test.z);

			std::cout << "Distance: " << Nz::Vector3f::Distance(entityPhys.GetPosition() + data.positionError, visualPosition) << std::endl;
			std::cout << "Rotation: " << std::atan2(testVec.GetLength(), test.w) << std::endl;
		}
	}

	void ServerMatchEntities::CopyState(std::size_t index, const Packets::ArenaState& arenaState)
	{
		Snapshot& snapshot = m_jitterBuffer[index];
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

		snapshot.stateId = arenaState.stateId;
		snapshot.isValid = true;
	}

	bool ServerMatchEntities::HandleNextSnapshot()
	{
		if (m_jitterBufferSize == 0)
			return false;

		auto& snapshot = m_jitterBuffer.front();

		bool isSnapshotValid = snapshot.isValid;
		if (isSnapshotValid)
		{
			ApplySnapshot(snapshot);
			snapshot.isValid = false;
		}

		std::rotate(m_jitterBuffer.begin(), m_jitterBuffer.begin() + 1, m_jitterBuffer.end());
		m_jitterBufferSize--;

		return isSnapshotValid;
	}

	void ServerMatchEntities::MarkStateAsLost(std::size_t first, std::size_t last)
	{
		for (std::size_t i = first; i < last; ++i)
			m_jitterBuffer[i].isValid = false;
	}

	void ServerMatchEntities::ResetSnapshots(const Packets::ArenaState& arenaState)
	{
		// Missed too many packets, drop everything and recreate
		CopyState(0, arenaState);
		m_jitterBufferSize = 1;
	}
}
