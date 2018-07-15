// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Systems/BroadcastSystem.hpp>
#include <Server/Components/SynchronizedComponent.hpp>
#include <Nazara/Core/MemoryHelper.hpp>
#include <Nazara/Physics3D/Collider3D.hpp>
#include <NDK/Components/CollisionComponent3D.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent3D.hpp>
#include <Server/ServerApplication.hpp>
#include <Server/Systems/InputSystem.hpp>
#include <cassert>

namespace ewn
{
	BroadcastSystem::BroadcastSystem(ServerApplication* app) :
	m_snapshotId(0),
	m_app(app)
	{
		Requires<Ndk::NodeComponent, SynchronizedComponent>();
		SetMaximumUpdateRate(30.f);
		SetUpdateOrder(100);
	}

	void BroadcastSystem::OnEntityRemoved(Ndk::Entity* entity)
	{
		m_movingEntities.Remove(entity);

		Ndk::EntityId entityId = entity->GetId();
		if (m_createdEntities.UnboundedTest(entityId))
			m_createdEntities.Reset(entityId);
		else
			m_deletedEntities.UnboundedSet(entityId);
	}

	void BroadcastSystem::OnEntityValidation(Ndk::Entity* entity, bool justAdded)
	{
		auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
		auto& syncComponent = entity->GetComponent<SynchronizedComponent>();

		if (entity->HasComponent<Ndk::PhysicsComponent3D>())
			m_movingEntities.Insert(entity);
		else
			m_movingEntities.Remove(entity);

		if (justAdded)
			m_createdEntities.UnboundedSet(entity->GetId());
	}

	void BroadcastSystem::OnUpdate(float /*elapsedTime*/)
	{
		static constexpr std::size_t HeaderSize = sizeof(Nz::UInt16) + 2 * sizeof(Nz::UInt64) + sizeof(Nz::UInt32);
		static constexpr std::size_t EntitySize = sizeof(Packets::ArenaState::Entity);
		static constexpr std::size_t EntityMaxSize = 1300;
		static constexpr std::size_t MaxEntityPerUpdate = EntityMaxSize / EntitySize;

		// Handle entities suppression
		if (m_deletedEntities.TestAny())
		{
			m_deletedEntitiesPacket.entities.clear();
			for (std::size_t entityId = m_deletedEntities.FindFirst(); entityId != m_deletedEntities.npos; entityId = m_deletedEntities.FindNext(entityId))
				m_deletedEntitiesPacket.entities.emplace_back(static_cast<Nz::UInt32>(entityId));

			m_deletedEntities.Clear();

			BroadcastEntitiesDestruction(this, m_deletedEntitiesPacket);

			m_destructionPacket++;
		}

		// Handle entities creation
		if (m_createdEntities.TestAny())
		{
			m_createdEntitiesPacket.entities.clear();
			for (std::size_t entityId = m_createdEntities.FindFirst(); entityId != m_createdEntities.npos; entityId = m_createdEntities.FindNext(entityId))
				AppendEntity(GetWorld().GetEntity(entityId), m_createdEntitiesPacket);

			m_createdEntities.Clear();

			BroadcastEntitiesCreation(this, m_createdEntitiesPacket);

			m_creationPacket++;
		}

		// Handle entities movement
		m_priorityQueue.clear();
		m_priorityQueue.reserve(m_movingEntities.size());

		for (const Ndk::EntityHandle& entities : m_movingEntities)
		{
			auto& entitySync = entities->GetComponent<SynchronizedComponent>();
			entitySync.AccumulatePriority();

			Nz::UInt16 priorityAccumulator = entitySync.GetPriorityAccumulator();
			if (priorityAccumulator == 0)
				continue;

			auto& priorityData = m_priorityQueue.emplace_back();
			priorityData.entity = entities;
			priorityData.priority = priorityAccumulator;
		}

		std::sort(m_priorityQueue.begin(), m_priorityQueue.end(), [](const EntityPriority& lhs, const EntityPriority& rhs)
		{
			return lhs.priority > rhs.priority;
		});

		// Fill our packet with at most MaxEntityPerUpdate entities, by priority order
		m_arenaStatePacket.stateId = m_snapshotId++;
		m_arenaStatePacket.serverTime = m_app->GetAppTime();

		std::size_t counter = 0;

		m_arenaStatePacket.entities.clear();
		for (const EntityPriority& priority : m_priorityQueue)
		{
			if (++counter > MaxEntityPerUpdate)
				break;

			auto& entityPhys = priority.entity->GetComponent<Ndk::PhysicsComponent3D>();
			auto& entitySync = priority.entity->GetComponent<SynchronizedComponent>();

			entitySync.ResetPriorityAccumulator();

			Packets::ArenaState::Entity entityData;
			entityData.id = priority.entity->GetId();
			entityData.angularVelocity = entityPhys.GetAngularVelocity();
			entityData.linearVelocity = entityPhys.GetLinearVelocity();
			entityData.position = entityPhys.GetPosition();
			entityData.rotation = entityPhys.GetRotation();

			m_arenaStatePacket.entities.emplace_back(std::move(entityData));
		}

		BroadcastStateUpdate(this, m_arenaStatePacket);
	}

	void BroadcastSystem::AppendEntity(Ndk::Entity* entity, Packets::CreateEntities& createPacket)
	{
		auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
		auto& syncComponent = entity->GetComponent<SynchronizedComponent>();

		auto& entityData = createPacket.entities.emplace_back();

		entityData.prefabId = Nz::UInt32(syncComponent.GetPrefabId());
		entityData.entityId = entity->GetId();
		entityData.position = nodeComponent.GetPosition();
		entityData.rotation = nodeComponent.GetRotation();
		entityData.visualName = syncComponent.GetName();

		if (entity->HasComponent<Ndk::PhysicsComponent3D>())
		{
			auto& physComponent = entity->GetComponent<Ndk::PhysicsComponent3D>();

			entityData.angularVelocity = physComponent.GetAngularVelocity();
			entityData.linearVelocity = physComponent.GetLinearVelocity();
		}
		else
		{
			entityData.angularVelocity = Nz::Vector3f::Zero();
			entityData.linearVelocity = Nz::Vector3f::Zero();
		}
	}

	void BroadcastSystem::CreateAllEntities(Packets::CreateEntities& packetVector)
	{
		for (const Ndk::EntityHandle& entity : GetEntities())
			AppendEntity(entity, packetVector);
	}

	Ndk::SystemIndex BroadcastSystem::systemIndex;
}
