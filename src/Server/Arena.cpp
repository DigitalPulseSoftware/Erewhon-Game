// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Arena.hpp>
#include <NDK/Components/CollisionComponent3D.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent3D.hpp>
#include <NDK/Systems/PhysicsSystem3D.hpp>
#include <Server/Player.hpp>
#include <Server/ServerApplication.hpp>
#include <Server/Components/PlayerControlledComponent.hpp>
#include <Server/Components/SynchronizedComponent.hpp>
#include <Server/Systems/BroadcastSystem.hpp>
#include <Server/Systems/SpaceshipSystem.hpp>
#include <cassert>

namespace ewn
{
	static constexpr bool sendServerGhosts = false;

	Arena::Arena(ServerApplication* app) :
	m_app(app),
	m_stateBroadcastAccumulator(0.f)
	{
		auto& broadcastSystem = m_world.AddSystem<BroadcastSystem>(app);
		broadcastSystem.BroadcastEntityCreation.Connect(this,    &Arena::OnBroadcastEntityCreation);
		broadcastSystem.BroadcastEntityDestruction.Connect(this, &Arena::OnBroadcastEntityDestruction);
		broadcastSystem.BroadcastStateUpdate.Connect(this,       &Arena::OnBroadcastStateUpdate);

		if (sendServerGhosts)
			broadcastSystem.SetMaximumUpdateRate(60.f);

		m_world.AddSystem<SpaceshipSystem>();

		// Earth entity
		m_attractionPoint = CreateEntity("earth", "The (small) Earth", Nz::Vector3f::Forward() * 50.f);
		CreateEntity("ball", "The (big) ball", Nz::Vector3f::Up() * 50.f);

		if constexpr (sendServerGhosts)
		{
			m_debugSocket.Create(Nz::NetProtocol_IPv4);
			m_debugSocket.EnableBroadcasting(true);
		}
	}

	const Ndk::EntityHandle& Arena::CreatePlayerSpaceship(Player* player)
	{
		assert(m_players.find(player) != m_players.end());

		const Ndk::EntityHandle& spaceship = CreateEntity("spaceship", player->GetName(), Nz::Vector3f::Zero());

		m_players[player] = spaceship;

		return spaceship;
	}

	const Ndk::EntityHandle& Arena::CreateProjectile(Player* owner, const Nz::Vector3f& position)
	{
		return CreateEntity("projectile", "Projectile de " + owner->GetName(), position);
	}

	void Arena::DispatchChatMessage(Player* player, const Nz::String& message)
	{
		Packets::ChatMessage chatPacket;
		chatPacket.message = message;

		for (auto& pair : m_players)
			pair.first->SendPacket(chatPacket);
	}

	void Arena::Update(float elapsedTime)
	{
		m_world.Update(elapsedTime);

		// Attraction
		/*if (m_attractionPoint)
		{
			constexpr float G = 6.6740831f / 10'000.f;

			Nz::Vector3f attractorPos = m_attractionPoint->GetComponent<Ndk::NodeComponent>().GetPosition();
			float attractorMass = 5'000.f;

			for (const Ndk::EntityHandle& entity : m_world.GetEntities())
			{
				if (entity->HasComponent<Ndk::PhysicsComponent3D>())
				{
					Nz::Vector3f entityPos = entity->GetComponent<Ndk::NodeComponent>().GetPosition();
					auto& phys = entity->GetComponent<Ndk::PhysicsComponent3D>();

					Nz::Vector3f dir = attractorPos - entityPos;
					float d2 = attractorPos.SquaredDistance(entityPos);

					phys.AddForce(dir * G * attractorMass * phys.GetMass() / d2);
				}
			}
		}*/

		m_stateBroadcastAccumulator += elapsedTime;
	}

	const Ndk::EntityHandle& Arena::CreateEntity(std::string type, std::string name, const Nz::Vector3f& position)
	{
		const Ndk::EntityHandle& newEntity = m_world.CreateEntity();

		if (type == "spaceship")
		{
			Nz::SphereCollider3DRef collider = Nz::SphereCollider3D::New(5.f);
			auto& collisionComponent = newEntity->AddComponent<Ndk::CollisionComponent3D>(collider);

			auto& physComponent = newEntity->AddComponent<Ndk::PhysicsComponent3D>();
			physComponent.SetMass(42.f);
			physComponent.SetAngularDamping(Nz::Vector3f(0.3f));
			physComponent.SetLinearDamping(0.25f);

			newEntity->AddComponent<Ndk::NodeComponent>().SetPosition(position);
			newEntity->AddComponent<PlayerControlledComponent>();
			newEntity->AddComponent<SynchronizedComponent>(type, name, true);
		}
		else if (type == "earth")
		{
			newEntity->AddComponent<Ndk::CollisionComponent3D>(Nz::SphereCollider3D::New(20.f));
			newEntity->AddComponent<Ndk::NodeComponent>().SetPosition(position);
			newEntity->AddComponent<SynchronizedComponent>(type, name, false);
		}
		else if (type == "ball")
		{
			constexpr float radius = 18.251904f / 2.f;

			newEntity->AddComponent<Ndk::CollisionComponent3D>(Nz::SphereCollider3D::New(radius));
			newEntity->AddComponent<Ndk::NodeComponent>().SetPosition(position);
			newEntity->AddComponent<SynchronizedComponent>(type, name, true);

			auto& physComponent = newEntity->AddComponent<Ndk::PhysicsComponent3D>();
			physComponent.SetMass(10.f);
			physComponent.SetPosition(position);
		}
		else if (type == "projectile")
		{
			constexpr float radius = 18.251904f / (2.f * 5.f);

			newEntity->AddComponent<Ndk::CollisionComponent3D>(Nz::SphereCollider3D::New(radius));
			newEntity->AddComponent<Ndk::NodeComponent>().SetPosition(position);
			newEntity->AddComponent<SynchronizedComponent>(type, name, true);

			auto& physComponent = newEntity->AddComponent<Ndk::PhysicsComponent3D>();
			physComponent.SetMass(10.f);
			physComponent.SetPosition(position);
		}

		return newEntity;
	}

	void Arena::HandlePlayerLeave(Player* player)
	{
		assert(m_players.find(player) != m_players.end());

		m_players.erase(player);
	}

	void Arena::HandlePlayerJoin(Player* player)
	{
		assert(m_players.find(player) == m_players.end());

		m_createEntityCache.clear();
		m_world.GetSystem<BroadcastSystem>().CreateAllEntities(m_createEntityCache);

		for (const auto& packet : m_createEntityCache)
			player->SendPacket(packet);

		m_players[player] = Ndk::EntityHandle::InvalidHandle;
	}

	void Arena::OnBroadcastEntityCreation(const BroadcastSystem* /*system*/, const Packets::CreateEntity& packet)
	{
		for (auto& pair : m_players)
			pair.first->SendPacket(packet);
	}

	void Arena::OnBroadcastEntityDestruction(const BroadcastSystem* /*system*/, const Packets::DeleteEntity& packet)
	{
		for (auto& pair : m_players)
			pair.first->SendPacket(packet);
	}

	void Arena::OnBroadcastStateUpdate(const BroadcastSystem* /*system*/, Packets::ArenaState& statePacket)
	{
		constexpr float stateBroadcastInterval = 1.f / 10.f;
		if (m_stateBroadcastAccumulator >= stateBroadcastInterval)
		{
			m_stateBroadcastAccumulator -= stateBroadcastInterval;

			for (auto& pair : m_players)
			{
				statePacket.lastProcessedInputTime = pair.first->GetLastInputProcessedTime();

				pair.first->SendPacket(statePacket);
			}
		}

		if constexpr (sendServerGhosts)
		{
			// Broadcast arena state over network, for testing purposes
			Nz::NetPacket debugState(1);
			Packets::Serialize(debugState, statePacket);

			Nz::IpAddress debugAddress = Nz::IpAddress::BroadcastIpV4;
			debugAddress.SetPort(2050);

			m_debugSocket.SendPacket(debugAddress, debugState);
		}
	}
}
