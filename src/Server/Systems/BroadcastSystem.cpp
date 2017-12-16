// Copyright (C) 2017 Jérôme Leclercq
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
#include <Server/Systems/SpaceshipSystem.hpp>
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

		Packets::DeleteEntity deletePacket;
		deletePacket.id = entity->GetId();

		BroadcastEntityDestruction(this, deletePacket);
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
		{
			Packets::CreateEntity createPacket;
			BuildCreateEntity(entity, createPacket);

			BroadcastEntityCreation(this, createPacket);
		}
	}

	void BroadcastSystem::OnUpdate(float /*elapsedTime*/)
	{
		static constexpr std::size_t HeaderSize = sizeof(Nz::UInt16) + 2 * sizeof(Nz::UInt64) + sizeof(Nz::UInt32);
		static constexpr std::size_t EntitySize = sizeof(Packets::ArenaState::Entity);
		static constexpr std::size_t EntityMaxSize = 1300;
		static constexpr std::size_t MaxEntityPerUpdate = EntityMaxSize / EntitySize;

		struct EntityPriority 
		{
			Ndk::Entity* entity;
			Nz::UInt16 priority;
		};

		// Allocate a temporary array on the stack to sort entities by their priority accumulator
		Nz::StackArray<EntityPriority> priorityQueue = NazaraStackAllocationNoInit(EntityPriority, m_movingEntities.size());

		std::size_t i = 0;
		for (const Ndk::EntityHandle& entities : m_movingEntities)
		{
			auto& entitySync = entities->GetComponent<SynchronizedComponent>();
			entitySync.AccumulatePriority();

			priorityQueue[i].entity = entities;
			priorityQueue[i].priority = entitySync.GetPriorityAccumulator();

			i++;
		}

		std::sort(priorityQueue.begin(), priorityQueue.end(), [](const EntityPriority& lhs, const EntityPriority& rhs)
		{
			return lhs.priority > rhs.priority;
		});

		// Fill our packet with at most MaxEntityPerUpdate entities, by priority order
		m_arenaStatePacket.stateId = m_snapshotId++;
		m_arenaStatePacket.serverTime = m_app->GetAppTime();

		std::size_t counter = 0;

		m_arenaStatePacket.entities.clear();
		for (const EntityPriority& priority : priorityQueue)
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

	void BroadcastSystem::BuildCreateEntity(Ndk::Entity* entity, Packets::CreateEntity& createPacket)
	{
		auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
		auto& syncComponent = entity->GetComponent<SynchronizedComponent>();

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
			createPacket.linearVelocity = Nz::Vector3f::Zero();
		}
	}

	void BroadcastSystem::CreateAllEntities(std::vector<Packets::CreateEntity>& packetVector)
	{
		for (const Ndk::EntityHandle& entity : GetEntities())
		{
			packetVector.emplace_back();
			BuildCreateEntity(entity, packetVector.back());
		}
	}

	Ndk::SystemIndex BroadcastSystem::systemIndex;
}
