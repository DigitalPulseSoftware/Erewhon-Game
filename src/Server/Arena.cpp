// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Arena.hpp>
#include <Nazara/Physics3D/PhysWorld3D.hpp>
#include <NDK/Components/CollisionComponent3D.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent3D.hpp>
#include <NDK/Systems/PhysicsSystem3D.hpp>
#include <Server/Player.hpp>
#include <Server/ServerApplication.hpp>
#include <Server/Components/HealthComponent.hpp>
#include <Server/Components/InputComponent.hpp>
#include <Server/Components/LifeTimeComponent.hpp>
#include <Server/Components/OwnerComponent.hpp>
#include <Server/Components/PlayerControlledComponent.hpp>
#include <Server/Components/ProjectileComponent.hpp>
#include <Server/Components/ScriptComponent.hpp>
#include <Server/Components/SynchronizedComponent.hpp>
#include <Server/Systems/BroadcastSystem.hpp>
#include <Server/Systems/LifeTimeSystem.hpp>
#include <Server/Systems/ScriptSystem.hpp>
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

		m_world.AddSystem<LifeTimeSystem>();
		m_world.AddSystem<ScriptSystem>(app);
		m_world.AddSystem<SpaceshipSystem>();

		Nz::PhysWorld3D& world = m_world.GetSystem<Ndk::PhysicsSystem3D>().GetWorld();
		int defaultMaterial = world.GetMaterial("default");
		m_projectileMaterial = world.CreateMaterial("laser");

		world.SetMaterialCollisionCallback(defaultMaterial, m_projectileMaterial, nullptr, [this](const Nz::RigidBody3D& firstBody, const Nz::RigidBody3D& secondBody)
		{
			return HandleProjectileCollision(firstBody, secondBody);
		});

		// Earth entity
		m_attractionPoint = CreateEntity("earth", "The (small) Earth", nullptr, Nz::Vector3f::Forward() * 50.f, Nz::Quaternionf::Identity());
		CreateEntity("ball", "The (big) ball", nullptr, Nz::Vector3f::Up() * 50.f, Nz::Quaternionf::Identity());

		const Ndk::EntityHandle& spaceship = CreateEntity("spaceship", "Bot", nullptr, Nz::Vector3f::Zero(), Nz::Quaternionf::Identity());
		spaceship->AddComponent<ScriptComponent>("test_script.lua");

		if constexpr (sendServerGhosts)
		{
			m_debugSocket.Create(Nz::NetProtocol_IPv4);
			m_debugSocket.EnableBroadcasting(true);
		}
	}

	const Ndk::EntityHandle& Arena::CreatePlayerSpaceship(Player* player)
	{
		assert(m_players.find(player) != m_players.end());

		const Ndk::EntityHandle& spaceship = CreateEntity("spaceship", player->GetName(), player, Nz::Vector3f::Zero(), Nz::Quaternionf::Identity());
		spaceship->AddComponent<PlayerControlledComponent>(player);

		m_players[player] = spaceship;

		return spaceship;
	}

	const Ndk::EntityHandle& Arena::CreateProjectile(Player* owner, const Ndk::EntityHandle& emitter, const Nz::Vector3f& position, const Nz::Quaternionf& rotation)
	{
		const Ndk::EntityHandle& projectile = CreateEntity("projectile", {}, owner, position, rotation);
		projectile->GetComponent<ProjectileComponent>().MarkAsHit(emitter);

		auto& projectilePhys = projectile->GetComponent<Ndk::PhysicsComponent3D>();
		projectilePhys.SetLinearVelocity(emitter->GetComponent<Ndk::NodeComponent>().GetForward() * 250.f);

		return projectile;
	}

	void Arena::DispatchChatMessage(Player* /*player*/, const Nz::String& message)
	{
		Packets::ChatMessage chatPacket;
		chatPacket.message = message.ToStdString();

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

	const Ndk::EntityHandle& Arena::CreateEntity(std::string type, std::string name, Player* owner, const Nz::Vector3f& position, const Nz::Quaternionf& rotation)
	{
		const Ndk::EntityHandle& newEntity = m_world.CreateEntity();

		if (type == "spaceship")
		{
			Nz::SphereCollider3DRef collider = Nz::SphereCollider3D::New(4.f);
			auto& collisionComponent = newEntity->AddComponent<Ndk::CollisionComponent3D>(collider);

			auto& physComponent = newEntity->AddComponent<Ndk::PhysicsComponent3D>();
			physComponent.SetMass(42.f);
			physComponent.SetAngularDamping(Nz::Vector3f(0.4f));
			physComponent.SetLinearDamping(0.25f);

			auto& healthComponent = newEntity->AddComponent<HealthComponent>(1000);
			healthComponent.OnDeath.Connect([this](HealthComponent* health, const Ndk::EntityHandle& attacker)
			{
				const Ndk::EntityHandle& entity = health->GetEntity();

				// Reset health and position
				health->Heal(health->GetMaxHealth());

				auto& node = entity->GetComponent<Ndk::PhysicsComponent3D>();
				node.SetPosition(Nz::Vector3f::Zero());

				if (entity->HasComponent<PlayerControlledComponent>() && attacker->HasComponent<OwnerComponent>())
				{
					auto& shipOwner = entity->GetComponent<PlayerControlledComponent>();
					auto& attackerOwner = attacker->GetComponent<OwnerComponent>();

					Player* shipOwnerPlayer = shipOwner.GetOwner();
					if (!shipOwnerPlayer)
						return;

					Player* attackerPlayer = attackerOwner.GetOwner();
					Nz::String attackerName = (attackerPlayer) ? attackerPlayer->GetName() : "<Disconnected>";

					DispatchChatMessage(nullptr, attackerName + " has destroyed " + shipOwnerPlayer->GetName());
				}
			});

			healthComponent.OnHealthChange.Connect([this](HealthComponent* health)
			{
				const Ndk::EntityHandle& entity = health->GetEntity();
				if (!entity->HasComponent<PlayerControlledComponent>())
					return;

				Player* owner = entity->GetComponent<PlayerControlledComponent>().GetOwner();
				if (!owner)
					return;

				Nz::UInt8 integrityPct = static_cast<Nz::UInt8>(Nz::Clamp(health->GetHealthPct() / 100.f * 255.f, 0.f, 255.f));

				Packets::IntegrityUpdate integrityPacket;
				integrityPacket.integrityValue = integrityPct;

				owner->SendPacket(integrityPacket);
			});

			newEntity->AddComponent<InputComponent>();
			newEntity->AddComponent<SynchronizedComponent>(type, name, true, 5);

			auto& node = newEntity->AddComponent<Ndk::NodeComponent>();
			node.SetPosition(position);
			node.SetRotation(rotation);
		}
		else if (type == "earth")
		{
			newEntity->AddComponent<Ndk::CollisionComponent3D>(Nz::SphereCollider3D::New(20.f));
			newEntity->AddComponent<Ndk::NodeComponent>().SetPosition(position);
			newEntity->AddComponent<SynchronizedComponent>(type, name, false, 0);
		}
		else if (type == "ball")
		{
			constexpr float radius = 18.251904f / 2.f;

			newEntity->AddComponent<Ndk::CollisionComponent3D>(Nz::SphereCollider3D::New(radius));
			newEntity->AddComponent<SynchronizedComponent>(type, name, true, 3);

			auto& node = newEntity->AddComponent<Ndk::NodeComponent>();
			node.SetPosition(position);
			node.SetRotation(rotation);

			auto& physComponent = newEntity->AddComponent<Ndk::PhysicsComponent3D>();
			physComponent.SetLinearDamping(0.05f);
			physComponent.SetMass(10.f);
			physComponent.SetPosition(position);
			physComponent.SetRotation(rotation);
		}
		else if (type == "projectile")
		{
			newEntity->AddComponent<Ndk::CollisionComponent3D>(Nz::CapsuleCollider3D::New(4.f, 0.5f, Nz::Vector3f::Zero(), Nz::EulerAnglesf(0.f, 90.f, 0.f)));
			newEntity->AddComponent<LifeTimeComponent>(10.f);
			newEntity->AddComponent<ProjectileComponent>(Nz::UInt16(50 + ((m_app->GetAppTime() % 21) - 10))); //< Aléatoire du pauvre
			newEntity->AddComponent<SynchronizedComponent>(type, name, true, 0);

			auto& node = newEntity->AddComponent<Ndk::NodeComponent>();
			node.SetPosition(position);
			node.SetRotation(rotation);

			auto& physComponent = newEntity->AddComponent<Ndk::PhysicsComponent3D>();
			physComponent.SetAngularDamping(Nz::Vector3f::Zero());
			physComponent.SetLinearDamping(0.f);
			physComponent.SetMass(1.f);
			physComponent.SetMaterial("laser");
			physComponent.SetPosition(position);
			physComponent.SetRotation(rotation);
		}

		if (owner)
			newEntity->AddComponent<OwnerComponent>(owner);

		return newEntity;
	}

	void Arena::HandlePlayerLeave(Player* player)
	{
		assert(m_players.find(player) != m_players.end());

		DispatchChatMessage(player, player->GetName() + " has left");

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

		DispatchChatMessage(player, player->GetName() + " has joined");
	}

	bool Arena::HandleProjectileCollision(const Nz::RigidBody3D& firstBody, const Nz::RigidBody3D& secondBody)
	{
		Ndk::EntityId laserEntityId = static_cast<Ndk::EntityId>(reinterpret_cast<std::ptrdiff_t>(firstBody.GetUserdata()));
		Ndk::EntityId hitEntityId = static_cast<Ndk::EntityId>(reinterpret_cast<std::ptrdiff_t>(secondBody.GetUserdata()));

		if (secondBody.GetMaterial() == m_projectileMaterial)
		{
			assert(firstBody.GetMaterial() != m_projectileMaterial);
			std::swap(laserEntityId, hitEntityId);
		}

		const Ndk::EntityHandle& projectile = m_world.GetEntity(laserEntityId);
		const Ndk::EntityHandle& hitEntity = m_world.GetEntity(hitEntityId);

		assert(projectile->HasComponent<OwnerComponent>() && projectile->HasComponent<ProjectileComponent>());

		ProjectileComponent& projectileComponent = projectile->GetComponent<ProjectileComponent>();
		if (projectileComponent.HasBeenHit(hitEntity))
			return false;

		projectileComponent.MarkAsHit(hitEntity);

		// Deal damage if entity has a health value
		if (hitEntity->HasComponent<HealthComponent>())
		{
			auto& health = hitEntity->GetComponent<HealthComponent>();
			health.Damage(projectileComponent.GetDamageValue(), projectile);
		}

		// Apply physics force
		if (hitEntity->HasComponent<Ndk::PhysicsComponent3D>())
		{
			auto& hitEntityPhys = hitEntity->GetComponent<Ndk::PhysicsComponent3D>();
			auto& projectilePhys = projectile->GetComponent<Ndk::PhysicsComponent3D>();

			Nz::Vector3f projectileForce = projectilePhys.GetLinearVelocity();
			float projectileSpeed;
			projectileForce.Normalize(&projectileSpeed);
			projectileForce = projectileForce * (projectileSpeed * projectileSpeed) / 2.f;

			hitEntityPhys.AddForce(projectileForce);
		}

		projectile->Kill(); //< Remember entity destruction is not immediate, we can still use it safely

		return false;
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
		constexpr float stateBroadcastInterval = 1.f / 30.f;
		if (m_stateBroadcastAccumulator >= stateBroadcastInterval)
		{
			m_stateBroadcastAccumulator -= stateBroadcastInterval;

			static Nz::UInt16 snapshotId = 0;
			statePacket.stateId = snapshotId++;

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
