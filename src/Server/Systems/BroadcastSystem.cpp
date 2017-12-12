// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Systems/BroadcastSystem.hpp>
#include <Server/Components/SynchronizedComponent.hpp>
#include <Nazara/Physics3D/Collider3D.hpp>
#include <NDK/Components/CollisionComponent3D.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent3D.hpp>
#include <Server/ServerApplication.hpp>
#include <Server/Systems/SpaceshipSystem.hpp>
#include <cassert>

namespace ewn
{
	BroadcastSystem::BroadcastSystem(ServerApplication* app) :
	m_snapshotId(0),
	m_app(app)
	{
		Requires<Ndk::NodeComponent, SynchronizedComponent>();
		SetMaximumUpdateRate(10.f);
	}

	void BroadcastSystem::CreateAllEntities(std::vector<Packets::CreateEntity>& packetVector)
	{
		for (const Ndk::EntityHandle& entity : GetEntities())
		{
			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			auto& syncComponent = entity->GetComponent<SynchronizedComponent>();

			Packets::CreateEntity createPacket;
			createPacket.entityType = syncComponent.GetType();
			createPacket.id = entity->GetId();
			createPacket.name = syncComponent.GetName();
			createPacket.position = nodeComponent.GetPosition();
			createPacket.rotation = nodeComponent.GetRotation();

			if (entity->HasComponent<Ndk::PhysicsComponent3D>())
			{
				auto& physComponent = entity->GetComponent<Ndk::PhysicsComponent3D>();

				createPacket.angularVelocity = physComponent.GetAngularVelocity();
				createPacket.linearVelocity = physComponent.GetLinearVelocity();
			}
			else
			{
				createPacket.angularVelocity = Nz::Vector3f::Zero();
				createPacket.linearVelocity  = Nz::Vector3f::Zero();
			}

			packetVector.emplace_back(std::move(createPacket));
		}
	}

	void BroadcastSystem::OnEntityRemoved(Ndk::Entity* entity)
	{
		m_movingEntities.Remove(entity);

		Packets::DeleteEntity deletePacket;
		deletePacket.id = entity->GetId();

		BroadcastEntityDestruction(this, deletePacket);
	}

	void BroadcastSystem::OnEntityValidation(Ndk::Entity * entity, bool justAdded)
	{
		auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
		auto& syncComponent = entity->GetComponent<SynchronizedComponent>();

		if (entity->HasComponent<Ndk::PhysicsComponent3D>())
			m_movingEntities.Insert(entity);
		else
			m_movingEntities.Remove(entity);

		if (justAdded)
		{
			Packets::CreateEntity createPacket;
			createPacket.entityType = syncComponent.GetType();
			createPacket.id = entity->GetId();
			createPacket.name = syncComponent.GetName();
			createPacket.position = nodeComponent.GetPosition();
			createPacket.rotation = nodeComponent.GetRotation();

			BroadcastEntityCreation(this, createPacket);
		}
	}

	void BroadcastSystem::OnUpdate(float /*elapsedTime*/)
	{
		m_arenaStatePacket.stateId = m_snapshotId++;
		m_arenaStatePacket.serverTime = m_app->GetAppTime();

		m_arenaStatePacket.entities.clear();
		for (const Ndk::EntityHandle& entity : m_movingEntities)
		{
			Ndk::PhysicsComponent3D& entityPhys = entity->GetComponent<Ndk::PhysicsComponent3D>();

			Packets::ArenaState::Entity entityData;
			entityData.id = entity->GetId();
			entityData.angularVelocity = entityPhys.GetAngularVelocity();
			entityData.linearVelocity = entityPhys.GetLinearVelocity();
			entityData.position = entityPhys.GetPosition();
			entityData.rotation = entityPhys.GetRotation();

			m_arenaStatePacket.entities.emplace_back(std::move(entityData));
		}

		BroadcastStateUpdate(this, m_arenaStatePacket);
	}

	Ndk::SystemIndex BroadcastSystem::systemIndex;
}
