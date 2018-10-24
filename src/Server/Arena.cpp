// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Arena.hpp>
#include <Nazara/Physics3D/PhysWorld3D.hpp>
#include <NDK/Components/CollisionComponent3D.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent3D.hpp>
#include <NDK/Systems/PhysicsSystem3D.hpp>
#include <NDK/LuaAPI.hpp>
#include <Server/Player.hpp>
#include <Server/ServerApplication.hpp>
#include <Server/Components/ArenaComponent.hpp>
#include <Server/Components/HealthComponent.hpp>
#include <Server/Components/InputComponent.hpp>
#include <Server/Components/LifeTimeComponent.hpp>
#include <Server/Components/NavigationComponent.hpp>
#include <Server/Components/OwnerComponent.hpp>
#include <Server/Components/PlayerControlledComponent.hpp>
#include <Server/Components/ProjectileComponent.hpp>
#include <Server/Components/SignatureComponent.hpp>
#include <Server/Components/ScriptComponent.hpp>
#include <Server/Components/SynchronizedComponent.hpp>
#include <Server/Scripting/ArenaInterface.hpp>
#include <Server/Systems/BroadcastSystem.hpp>
#include <Server/Systems/LifeTimeSystem.hpp>
#include <Server/Systems/NavigationSystem.hpp>
#include <Server/Systems/ScriptSystem.hpp>
#include <Server/Systems/InputSystem.hpp>
#include <cassert>
#include <stdexcept>

namespace ewn
{
	static constexpr bool sendServerGhosts = false;

	Arena::Arena(ServerApplication* app, std::string name, std::string scriptName) :
	m_name(std::move(name)),
	m_scriptName(std::move(scriptName)),
	m_app(app)
	{
		auto& broadcastSystem = m_world.AddSystem<BroadcastSystem>(m_app);
		broadcastSystem.BroadcastEntitiesCreation.Connect(this,    &Arena::OnBroadcastEntitiesCreation);
		broadcastSystem.BroadcastEntitiesDestruction.Connect(this, &Arena::OnBroadcastEntitiesDestruction);
		broadcastSystem.BroadcastStateUpdate.Connect(this,       &Arena::OnBroadcastStateUpdate);

		if (sendServerGhosts)
			broadcastSystem.SetMaximumUpdateRate(60.f);

		m_world.GetSystem<Ndk::PhysicsSystem3D>().GetWorld().SetThreadCount(0);

		m_world.AddSystem<InputSystem>();
		m_world.AddSystem<LifeTimeSystem>();
		m_world.AddSystem<NavigationSystem>(m_app);
		m_world.AddSystem<ScriptSystem>(m_app, this);

		Nz::PhysWorld3D& world = m_world.GetSystem<Ndk::PhysicsSystem3D>().GetWorld();
		int defaultMaterial = world.GetMaterial("default");
		m_plasmaMaterial = world.CreateMaterial("plasma");
		m_torpedoMaterial = world.CreateMaterial("torpedo");

		world.SetMaterialCollisionCallback(defaultMaterial, defaultMaterial, nullptr, [this](const Nz::RigidBody3D& firstBody, const Nz::RigidBody3D& secondBody)
		{
			return HandleDefaultDefaultCollision(firstBody, secondBody);
		});

		world.SetMaterialCollisionCallback(m_plasmaMaterial, m_plasmaMaterial, nullptr, [this](const Nz::RigidBody3D& firstBody, const Nz::RigidBody3D& secondBody)
		{
			return false;
		});

		world.SetMaterialCollisionCallback(defaultMaterial, m_plasmaMaterial, nullptr, [this](const Nz::RigidBody3D& firstBody, const Nz::RigidBody3D& secondBody)
		{
			return HandlePlasmaProjectileCollision(firstBody, secondBody);
		});

		world.SetMaterialCollisionCallback(defaultMaterial, m_torpedoMaterial, nullptr, [this](const Nz::RigidBody3D& firstBody, const Nz::RigidBody3D& secondBody)
		{
			return HandleTorpedoProjectileCollision(firstBody, secondBody);
		});

		LoadScript(m_scriptName);

		Reset();

		if constexpr (sendServerGhosts)
		{
			m_debugSocket.Create(Nz::NetProtocol_IPv4);
			m_debugSocket.EnableBroadcasting(true);
		}
	}

	Arena::~Arena()
	{
		m_world.Clear();
	}

	const Ndk::EntityHandle& Arena::CreatePlasmaProjectile(Player* owner, const Ndk::EntityHandle& emitter, const Nz::Vector3f& position, const Nz::Quaternionf& rotation)
	{
		const Ndk::EntityHandle& projectile = CreateEntity("plasmabeam", {}, owner, position, rotation);
		projectile->GetComponent<ProjectileComponent>().MarkAsHit(emitter);

		auto& projectilePhys = projectile->GetComponent<Ndk::PhysicsComponent3D>();
		projectilePhys.SetLinearVelocity(emitter->GetComponent<Ndk::NodeComponent>().GetForward() * 250.f);

		return projectile;
	}

	const Ndk::EntityHandle& Arena::CreateTorpedo(Player* owner, const Ndk::EntityHandle & emitter, const Nz::Vector3f & position, const Nz::Quaternionf & rotation)
	{
		const Ndk::EntityHandle& projectile = CreateEntity("torpedo", {}, owner, position, rotation);
		projectile->GetComponent<ProjectileComponent>().MarkAsHit(emitter);

		auto& projectilePhys = projectile->GetComponent<Ndk::PhysicsComponent3D>();
		projectilePhys.SetLinearVelocity(emitter->GetComponent<Ndk::NodeComponent>().GetForward() * 50.f);

		return projectile;
	}

	Player* Arena::FindPlayerByName(const std::string& name) const
	{
		for (Player* player : m_players)
		{
			if (player->GetName() == name)
				return player;
		}

		return nullptr;
	}

	void Arena::HandleChatMessage(Player* sender, const std::string& message)
	{
		bool shouldPrintMessage = true;
		if (m_script.GetGlobal("OnPlayerChat") == Nz::LuaType_Function)
		{
			m_script.Push(sender);
			m_script.Push(message);

			if (!m_script.Call(2, 1))
				std::cerr << "An error occurred during OnPlayerChat call: " << m_script.GetLastError() << std::endl;

			shouldPrintMessage = m_script.ToBoolean(-1);
		}
		else
			m_script.Pop();

		if (!shouldPrintMessage)
			return;

		static constexpr std::size_t MaxChatLine = 255;

		Nz::String completeMessage = sender->GetName() + ": " + message;
		if (completeMessage.GetSize() > MaxChatLine)
		{
			completeMessage.Resize(MaxChatLine - 3, Nz::String::HandleUtf8);
			completeMessage += "...";
		}

		PrintChatMessage(completeMessage.ToStdString());
	}

	void Arena::PrintChatMessage(const std::string& message)
	{
		std::cout << "(" << m_name << ") " << message << std::endl;

		Packets::ChatMessage chatPacket;
		chatPacket.message = message;

		for (Player* player : m_players)
			player->SendPacket(chatPacket);
	}

	void Arena::Reload()
	{
		LoadScript(m_scriptName);
		Reset();
	}

	void Arena::Reset()
	{
		for (Player* player : m_players)
			player->ClearBots();

		m_world.Clear();

		m_world.CreateEntity(); //< Reserve entity #0

		if (m_script.GetGlobal("OnReset") == Nz::LuaType_Function)
		{
			if (!m_script.Call(0))
				std::cerr << "An error occurred during OnReset call: " << m_script.GetLastError() << std::endl;
		}
		else
			m_script.Pop();
	}

	void Arena::SpawnFleet(Player* owner, const std::string& fleetName)
	{
		Nz::Vector3f spawnPos;
		Nz::Quaternionf spawnRot;
		if (const Ndk::EntityHandle& spaceship = owner->GetControlledEntity(); spaceship != Ndk::EntityHandle::InvalidHandle)
		{
			Ndk::NodeComponent& spaceshipNode = spaceship->GetComponent<Ndk::NodeComponent>();

			spawnRot = spaceshipNode.GetRotation();
			spawnPos = spaceshipNode.GetPosition() + spawnRot * Nz::Vector3f::Down() * 15.f;
		}
		else
		{
			spawnPos = Nz::Vector3f::Zero();
			spawnRot = Nz::Quaternionf::Identity();
		}

		SpawnFleet(owner, fleetName, spawnPos, spawnRot);
	}

	void Arena::SpawnFleet(Player* owner, const std::string& fleetName, const Nz::Vector3f& spawnPos, const Nz::Quaternionf& spawnRot)
	{
		owner->GetFleetData(fleetName, [this, fleetName, spawnPos, spawnRot, sessionId = owner->GetSessionId()](bool found, const Player::FleetData& fleet)
		{
			Player* ply = m_app->GetPlayerBySession(sessionId);
			if (!ply)
				return;

			if (!found)
			{
				ply->PrintMessage("Fleet " + fleetName + " not found");
				return;
			}

			for (const auto& spaceship : fleet.spaceships)
			{
				const auto& spaceshipTypeData = fleet.spaceshipTypes[spaceship.spaceshipType];

				Nz::Vector3f position = spawnPos + spawnRot * spaceship.position;
				SpawnSpaceship(ply, spaceshipTypeData.script, spaceshipTypeData.hullId, spaceshipTypeData.modules, position, spawnRot);
			}
		});
	}

	void Arena::SpawnSpaceship(Player* owner, const std::string& spaceshipName, const Nz::Vector3f& position, const Nz::Quaternionf& rotation)
	{
		m_app->GetGlobalDatabase().ExecuteStatement("FindSpaceshipByOwnerIdAndName", { owner->GetDatabaseId(), spaceshipName }, [=, sessionId = owner->GetSessionId()](DatabaseResult& result)
		{
			if (!result)
				std::cerr << "Find spaceship query failed: " << result.GetLastErrorMessage() << std::endl;

			Player* ply = m_app->GetPlayerBySession(sessionId);
			if (!ply)
				return;

			if (!result)
			{
				ply->PrintMessage("Failed to spawn spaceship \"" + spaceshipName + "\", please contact an admin");
				return;
			}

			if (result.GetRowCount() == 0)
			{
				ply->PrintMessage("You have no spaceship named \"" + spaceshipName + "\"");
				return;
			}

			Nz::Int32 spaceshipId = std::get<Nz::Int32>(result.GetValue(0));
			std::string code = std::get<std::string>(result.GetValue(1));
			Nz::Int32 spaceshipHullId = std::get<Nz::Int32>(result.GetValue(2));

			SpawnSpaceship(ply, spaceshipId, std::move(code), spaceshipHullId, position, rotation);
		});
	}

	void Arena::SpawnSpaceship(Player* owner, Nz::Int32 spaceshipId, const Nz::Vector3f& position, const Nz::Quaternionf& rotation)
	{
		m_app->GetGlobalDatabase().ExecuteStatement("FindSpaceshipByIdAndOwnerId", { spaceshipId, owner->GetDatabaseId() }, [=, sessionId = owner->GetSessionId()](DatabaseResult& result)
		{
			if (!result)
				std::cerr << "Find spaceship query failed: " << result.GetLastErrorMessage() << std::endl;

			Player* ply = m_app->GetPlayerBySession(sessionId);
			if (!ply)
				return;

			if (!result || result.GetRowCount() == 0)
			{
				ply->PrintMessage("Failed to spawn spaceship id " + std::to_string(spaceshipId) + ", please contact an admin");
				return;
			}

			std::string code = std::get<std::string>(result.GetValue(1));
			Nz::Int32 spaceshipHullId = std::get<Nz::Int32>(result.GetValue(2));

			SpawnSpaceship(ply, spaceshipId, std::move(code), spaceshipHullId, position, rotation);
		});
	}

	void Arena::Update(float elapsedTime)
	{
		m_world.Update(elapsedTime);
		for (Player* player : m_players)
			player->Update(elapsedTime);

		if (m_script.GetGlobal("OnUpdate") == Nz::LuaType_Function)
		{
			m_script.Push(elapsedTime);

			if (!m_script.Call(1, 0))
				std::cerr << "An error occurred during OnUpdate call: " << m_script.GetLastError() << std::endl;
		}
		else
			m_script.Pop();
	}

	const Ndk::EntityHandle& Arena::CreateEntity(std::string type, std::string name, Player* owner, const Nz::Vector3f& position, const Nz::Quaternionf& rotation)
	{
		const Ndk::EntityHandle& newEntity = m_world.CreateEntity();

		if (type == "earth")
		{
			constexpr float radius = 50.f;

			auto collider = Nz::SphereCollider3D::New(radius);

			newEntity->AddComponent<Ndk::CollisionComponent3D>(Nz::SphereCollider3D::New(radius));
			newEntity->AddComponent<Ndk::NodeComponent>().SetPosition(position);
			newEntity->AddComponent<SignatureComponent>(newEntity->GetId(), 0.000035, collider->GetRadius(), collider->ComputeVolume());
			newEntity->AddComponent<SynchronizedComponent>(0, type, name, false, 0);
		}
		else if (type == "light")
		{
			newEntity->AddComponent<SynchronizedComponent>(1, type, name, false, 0);

			auto& node = newEntity->AddComponent<Ndk::NodeComponent>();
			node.SetPosition(position);
			node.SetRotation(rotation);
		}
		else if (type == "ball")
		{
			constexpr float radius = 18.251904f / 2.f;

			auto collider = Nz::SphereCollider3D::New(radius);

			newEntity->AddComponent<Ndk::CollisionComponent3D>(collider);
			newEntity->AddComponent<SignatureComponent>(newEntity->GetId(), 0.0, collider->GetRadius(), collider->ComputeVolume());
			newEntity->AddComponent<SynchronizedComponent>(4, type, name, true, 3);

			auto& node = newEntity->AddComponent<Ndk::NodeComponent>();
			node.SetPosition(position);
			node.SetRotation(rotation);

			auto& physComponent = newEntity->AddComponent<Ndk::PhysicsComponent3D>();
			physComponent.SetLinearDamping(0.05f);
			physComponent.SetMass(100.f);
			physComponent.SetPosition(position);
			physComponent.SetRotation(rotation);
		}
		else if (type == "plasmabeam")
		{
			auto collider = Nz::CapsuleCollider3D::New(4.f, 0.5f, Nz::Vector3f::Zero(), Nz::EulerAnglesf(0.f, 90.f, 0.f));

			newEntity->AddComponent<Ndk::CollisionComponent3D>(collider);
			newEntity->AddComponent<LifeTimeComponent>(10.f);
			newEntity->AddComponent<ProjectileComponent>(Nz::UInt16(50 + ((m_app->GetAppTime() % 21) - 10))); //< Aléatoire du pauvre
			newEntity->AddComponent<SignatureComponent>(newEntity->GetId(), 10'000.0, collider->ComputeAABB().GetRadius(), collider->ComputeVolume());
			newEntity->AddComponent<SynchronizedComponent>(2, type, name, true, 0);

			auto& node = newEntity->AddComponent<Ndk::NodeComponent>();
			node.SetPosition(position);
			node.SetRotation(rotation);

			auto& physComponent = newEntity->AddComponent<Ndk::PhysicsComponent3D>();
			physComponent.SetAngularDamping(Nz::Vector3f::Zero());
			physComponent.SetLinearDamping(0.f);
			physComponent.SetMass(1.f);
			physComponent.SetMaterial("plasma");
			physComponent.SetPosition(position);
			physComponent.SetRotation(rotation);
		}
		else if (type == "torpedo")
		{
			auto collider = Nz::SphereCollider3D::New(3.f);

			newEntity->AddComponent<Ndk::CollisionComponent3D>(collider);
			newEntity->AddComponent<LifeTimeComponent>(30.f);
			newEntity->AddComponent<ProjectileComponent>(200);
			newEntity->AddComponent<SignatureComponent>(newEntity->GetId(), 1'000.0, collider->GetRadius(), collider->ComputeVolume());
			newEntity->AddComponent<SynchronizedComponent>(3, type, name, true, 0);

			auto& node = newEntity->AddComponent<Ndk::NodeComponent>();
			node.SetPosition(position);
			node.SetRotation(rotation);

			auto& physComponent = newEntity->AddComponent<Ndk::PhysicsComponent3D>();
			physComponent.SetAngularDamping(Nz::Vector3f::Zero());
			physComponent.SetLinearDamping(0.f);
			physComponent.SetMass(1.f);
			physComponent.SetMaterial("torpedo");
			physComponent.SetPosition(position);
			physComponent.SetRotation(rotation);
		}

		newEntity->AddComponent<ArenaComponent>(*this);

		if (owner)
			newEntity->AddComponent<OwnerComponent>(owner);

		return newEntity;
	}

	const Ndk::EntityHandle& Arena::CreateSpaceship(std::string name, Player* owner, std::size_t spaceshipHullId, const Nz::Vector3f& position, const Nz::Quaternionf& rotation)
	{
		const Ndk::EntityHandle& newEntity = m_world.CreateEntity();

		std::size_t collisionMeshId = m_app->GetSpaceshipHullStore().GetEntryCollisionMeshId(spaceshipHullId);
		Nz::Collider3DRef collider = m_app->GetCollisionMeshStore().GetEntryCollider(collisionMeshId);
		assert(collider);

		auto& collisionComponent = newEntity->AddComponent<Ndk::CollisionComponent3D>(collider);

		auto& physComponent = newEntity->AddComponent<Ndk::PhysicsComponent3D>();
		physComponent.SetMass(42.f);
		physComponent.SetAngularDamping(Nz::Vector3f(0.4f));
		physComponent.SetLinearDamping(0.25f);
		physComponent.SetPosition(position);
		physComponent.SetRotation(rotation);

		auto& healthComponent = newEntity->AddComponent<HealthComponent>(1000);
		healthComponent.OnDeath.Connect([this](HealthComponent* health, const Ndk::EntityHandle& attacker)
		{
			const Ndk::EntityHandle& entity = health->GetEntity();

			if (entity->HasComponent<PlayerControlledComponent>())
			{
				auto& shipOwner = entity->GetComponent<PlayerControlledComponent>();

				Player* shipOwnerPlayer = shipOwner.GetOwner();
				if (!shipOwnerPlayer)
					return;

				if (m_script.GetGlobal("OnPlayerDeath") == Nz::LuaType_Function)
				{
					m_script.Push(shipOwnerPlayer);

					if (!m_script.Call(1))
						std::cerr << "An error occurred during OnPlayerDeath call: " << m_script.GetLastError() << std::endl;
				}
				else
					m_script.Pop();

				if (attacker->HasComponent<OwnerComponent>())
				{
					auto& attackerOwner = attacker->GetComponent<OwnerComponent>();

					Player* attackerPlayer = attackerOwner.GetOwner();
					std::string attackerName = (attackerPlayer) ? attackerPlayer->GetName() : "<Disconnected>";

					PrintChatMessage(attackerName + " has destroyed " + shipOwnerPlayer->GetName());
				}
			}

			Ndk::NodeComponent& entityNode = entity->GetComponent<Ndk::NodeComponent>();

			Packets::InstantiateParticleSystem particlePacket;
			particlePacket.particleSystemId = 0; //< Explosion
			particlePacket.position = entityNode.GetPosition();
			particlePacket.rotation = entityNode.GetRotation();
			particlePacket.scale = Nz::Vector3f(1.f);
			BroadcastPacket(particlePacket);

			Packets::PlaySound soundPacket;
			soundPacket.position = entityNode.GetPosition();
			soundPacket.soundId = 2;
			BroadcastPacket(soundPacket);

			entity->Kill();
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

		Nz::Int64 signature;
		if (owner)
			signature = std::hash<std::string>()(owner->GetName() + std::to_string(newEntity->GetId()));
		else
			signature = std::hash<std::string>()("rogue" + std::to_string(newEntity->GetId()));

		newEntity->AddComponent<InputComponent>();
		newEntity->AddComponent<SignatureComponent>(signature, 42.0, collider->ComputeAABB().GetRadius(), collider->ComputeVolume());
		newEntity->AddComponent<SynchronizedComponent>((spaceshipHullId == 1) ? 5 : 6, "spaceship", name, true, 5);

		auto& node = newEntity->AddComponent<Ndk::NodeComponent>();
		node.SetPosition(position);
		node.SetRotation(rotation);

		newEntity->AddComponent<ArenaComponent>(*this);

		if (owner)
			newEntity->AddComponent<OwnerComponent>(owner);

		return newEntity;
	}

	bool Arena::LoadScript(std::string fileName)
	{
		m_script = Nz::LuaInstance();
		m_script.LoadLibraries();

		Ndk::LuaAPI::RegisterClasses(m_script);
		ArenaInterface::Register(m_script);

		m_script.Push(this);
		m_script.SetGlobal("Arena");

		if (!m_script.ExecuteFromFile(fileName))
		{
			std::cerr << "Failed to execute arena script: " + m_script.GetLastError() << std::endl;
			return false;
		}

		return true;
	}

	void Arena::HandlePlayerLeave(Player* player)
	{
		assert(m_players.find(player) != m_players.end());

		if (m_script.GetGlobal("OnPlayerLeave") == Nz::LuaType_Function)
		{
			m_script.Push(player);

			if (!m_script.Call(1))
				std::cerr << "An error occurred during OnPlayerLeave call: " << m_script.GetLastError() << std::endl;
		}
		else
			m_script.Pop();

		player->ClearControlledEntity();
		m_players.erase(player);
	}

	void Arena::HandlePlayerJoin(Player* player)
	{
		assert(m_players.find(player) == m_players.end());

		SendArenaData(player);

		m_createEntitiesCache.entities.clear();
		m_world.GetSystem<BroadcastSystem>().CreateAllEntities(m_createEntitiesCache);

		player->SendPacket(m_createEntitiesCache);

		m_players.insert(player);

		if (m_script.GetGlobal("OnPlayerJoined") == Nz::LuaType_Function)
		{
			m_script.Push(player);

			if (!m_script.Call(1))
				std::cerr << "An error occurred during OnPlayerJoined call: " << m_script.GetLastError() << std::endl;
		}
		else
			m_script.Pop();
	}

	void Arena::SendArenaData(Player* player)
	{
		Packets::ArenaParticleSystems arenaParticleSystems;
		arenaParticleSystems.startId = 0;

		arenaParticleSystems.particleSystems.emplace_back();
		arenaParticleSystems.particleSystems.back().particleGroups.emplace_back();
		arenaParticleSystems.particleSystems.back().particleGroups.back().particleGroupNameId = m_app->GetNetworkStringStore().GetStringIndex("explosion_flare");
		arenaParticleSystems.particleSystems.back().particleGroups.emplace_back();
		arenaParticleSystems.particleSystems.back().particleGroups.back().particleGroupNameId = m_app->GetNetworkStringStore().GetStringIndex("explosion_fire");
		arenaParticleSystems.particleSystems.back().particleGroups.emplace_back();
		arenaParticleSystems.particleSystems.back().particleGroups.back().particleGroupNameId = m_app->GetNetworkStringStore().GetStringIndex("explosion_smoke");
		arenaParticleSystems.particleSystems.back().particleGroups.emplace_back();
		arenaParticleSystems.particleSystems.back().particleGroups.back().particleGroupNameId = m_app->GetNetworkStringStore().GetStringIndex("explosion_wave");

		player->SendPacket(arenaParticleSystems);

		Packets::ArenaSounds arenaSoundsPacket;
		arenaSoundsPacket.startId = 0;

		arenaSoundsPacket.sounds.emplace_back();
		arenaSoundsPacket.sounds.back().filePath = "sounds/laserTurretlow.ogg";

		arenaSoundsPacket.sounds.emplace_back();
		arenaSoundsPacket.sounds.back().filePath = "sounds/torpedo_loop.wav";

		arenaSoundsPacket.sounds.emplace_back();
		arenaSoundsPacket.sounds.back().filePath = "sounds/spaceship_explosion.wav";

		arenaSoundsPacket.sounds.emplace_back();
		arenaSoundsPacket.sounds.back().filePath = "sounds/plasmabeam_loop.wav";

		player->SendPacket(arenaSoundsPacket);

		Packets::ArenaPrefabs arenaPrefabsPacket;
		arenaPrefabsPacket.startId = 0;

		// Earth
		arenaPrefabsPacket.prefabs.emplace_back();
		arenaPrefabsPacket.prefabs.back().visualEffects.emplace_back();
		arenaPrefabsPacket.prefabs.back().visualEffects.back().effectNameId = m_app->GetNetworkStringStore().GetStringIndex("earth");
		arenaPrefabsPacket.prefabs.back().visualEffects.back().position = Nz::Vector3f::Zero();
		arenaPrefabsPacket.prefabs.back().visualEffects.back().rotation = Nz::Quaternionf::Identity();
		arenaPrefabsPacket.prefabs.back().visualEffects.back().scale = Nz::Vector3f::Unit();

		// Light
		arenaPrefabsPacket.prefabs.emplace_back();
		arenaPrefabsPacket.prefabs.back().visualEffects.emplace_back();
		arenaPrefabsPacket.prefabs.back().visualEffects.back().effectNameId = m_app->GetNetworkStringStore().GetStringIndex("light");
		arenaPrefabsPacket.prefabs.back().visualEffects.back().position = Nz::Vector3f::Zero();
		arenaPrefabsPacket.prefabs.back().visualEffects.back().rotation = Nz::Quaternionf::Identity();
		arenaPrefabsPacket.prefabs.back().visualEffects.back().scale = Nz::Vector3f::Unit();

		// Plasma beam
		arenaPrefabsPacket.prefabs.emplace_back();

		arenaPrefabsPacket.prefabs.back().sounds.emplace_back();
		arenaPrefabsPacket.prefabs.back().sounds.back().position = Nz::Vector3f::Zero();
		arenaPrefabsPacket.prefabs.back().sounds.back().soundId = 3;

		arenaPrefabsPacket.prefabs.back().visualEffects.emplace_back();
		arenaPrefabsPacket.prefabs.back().visualEffects.back().effectNameId = m_app->GetNetworkStringStore().GetStringIndex("plasmabeam");
		arenaPrefabsPacket.prefabs.back().visualEffects.back().position = Nz::Vector3f::Zero();
		arenaPrefabsPacket.prefabs.back().visualEffects.back().rotation = Nz::Quaternionf::Identity();
		arenaPrefabsPacket.prefabs.back().visualEffects.back().scale = Nz::Vector3f::Unit();

		// Torpedo
		arenaPrefabsPacket.prefabs.emplace_back();

		arenaPrefabsPacket.prefabs.back().sounds.emplace_back();
		arenaPrefabsPacket.prefabs.back().sounds.back().position = Nz::Vector3f::Zero();
		arenaPrefabsPacket.prefabs.back().sounds.back().soundId = 1;

		arenaPrefabsPacket.prefabs.back().visualEffects.emplace_back();
		arenaPrefabsPacket.prefabs.back().visualEffects.back().effectNameId = m_app->GetNetworkStringStore().GetStringIndex("torpedo");
		arenaPrefabsPacket.prefabs.back().visualEffects.back().position = Nz::Vector3f::Zero();
		arenaPrefabsPacket.prefabs.back().visualEffects.back().rotation = Nz::Quaternionf::Identity();
		arenaPrefabsPacket.prefabs.back().visualEffects.back().scale = Nz::Vector3f::Unit();

		/*arenaPrefabsPacket.prefabs.back().sounds.emplace_back();
		arenaPrefabsPacket.prefabs.back().sounds.back().soundId = 1;
		arenaPrefabsPacket.prefabs.back().sounds.back().position = Nz::Vector3f::Zero();*/

		// Ball
		arenaPrefabsPacket.prefabs.emplace_back();
		arenaPrefabsPacket.prefabs.back().models.emplace_back();
		arenaPrefabsPacket.prefabs.back().models.back().modelId = m_app->GetNetworkStringStore().GetStringIndex("ball/ball.obj");
		arenaPrefabsPacket.prefabs.back().models.back().position = Nz::Vector3f::Zero();
		arenaPrefabsPacket.prefabs.back().models.back().rotation = Nz::Quaternionf::Identity();
		arenaPrefabsPacket.prefabs.back().models.back().scale = Nz::Vector3f::Unit();

		// Spaceship
		arenaPrefabsPacket.prefabs.emplace_back();
		arenaPrefabsPacket.prefabs.back().models.emplace_back();
		arenaPrefabsPacket.prefabs.back().models.back().modelId = m_app->GetNetworkStringStore().GetStringIndex("spaceship/spaceship.obj");
		arenaPrefabsPacket.prefabs.back().models.back().position = Nz::Vector3f::Zero();
		arenaPrefabsPacket.prefabs.back().models.back().rotation = Nz::EulerAnglesf(0.f, 90.f, 0.f);
		arenaPrefabsPacket.prefabs.back().models.back().scale = Nz::Vector3f(0.01f);

		// Space Frigate
		arenaPrefabsPacket.prefabs.emplace_back();
		arenaPrefabsPacket.prefabs.back().models.emplace_back();
		arenaPrefabsPacket.prefabs.back().models.back().modelId = m_app->GetNetworkStringStore().GetStringIndex("space_frigate_6/space_frigate_6.obj");
		arenaPrefabsPacket.prefabs.back().models.back().position = Nz::Vector3f::Zero();
		arenaPrefabsPacket.prefabs.back().models.back().rotation = Nz::EulerAnglesf(0.f, 90.f, 0.f);
		arenaPrefabsPacket.prefabs.back().models.back().scale = Nz::Vector3f(0.1f);

		player->SendPacket(arenaPrefabsPacket);
	}

	void Arena::SpawnSpaceship(Player* owner, Nz::Int32 spaceshipId, std::string code, std::size_t spaceshipHullId, const Nz::Vector3f& position, const Nz::Quaternionf& rotation)
	{
		m_app->GetGlobalDatabase().ExecuteStatement("FindSpaceshipModulesBySpaceshipId", { spaceshipId }, [this, position, rotation, sessionId = owner->GetSessionId(), spaceshipHullId, spaceshipCode = std::move(code)](DatabaseResult& result)
		{
			if (!result)
				std::cerr << "Find spaceship modules failed: " << result.GetLastErrorMessage() << std::endl;

			Player* ply = m_app->GetPlayerBySession(sessionId);
			if (!ply)
				return;

			if (!result)
			{
				ply->PrintMessage("Server: Failed to retrieve spaceship modules, please contact an administrator");
				return;
			}

			std::size_t moduleCount = result.GetRowCount();

			std::vector<std::size_t> moduleIds(moduleCount);
			try
			{
				for (std::size_t i = 0; i < moduleCount; ++i)
					moduleIds[i] = static_cast<std::size_t>(std::get<Nz::Int32>(result.GetValue(0, i)));
			}
			catch (const std::exception& e)
			{
				std::cerr << "Failed to retrieve spaceship modules: " << e.what() << std::endl;

				ply->PrintMessage("Server: Failed to retrieve spaceship modules, please contact an administrator");
				return;
			}

			SpawnSpaceship(ply, std::move(spaceshipCode), spaceshipHullId, moduleIds, position, rotation);
		});
	}

	const Ndk::EntityHandle& Arena::SpawnSpaceship(Player* owner, std::string code, std::size_t spaceshipHullId, const std::vector<std::size_t>& modules, const Nz::Vector3f& position, const Nz::Quaternionf& rotation)
	{
		assert(owner);

		const Ndk::EntityHandle& spaceship = CreateSpaceship("Bot (" + owner->GetName() + ')', owner, spaceshipHullId, position, rotation);
		ScriptComponent& botScript = spaceship->AddComponent<ScriptComponent>();
		if (!botScript.Initialize(m_app, modules))
		{
			owner->PrintMessage("Server: Failed to initialize bot, please contact an administrator");
			return spaceship;
		}

		Nz::String lastError;
		if (botScript.Execute(std::move(code), &lastError))
			owner->PrintMessage("Server: Script loaded with success");
		else
			owner->PrintMessage("Server: Failed to execute script: " + lastError.ToStdString());

		return spaceship;
	}

	bool Arena::HandleDefaultDefaultCollision(const Nz::RigidBody3D& firstBody, const Nz::RigidBody3D& secondBody)
	{
		Ndk::EntityId firstEntityId = static_cast<Ndk::EntityId>(reinterpret_cast<std::ptrdiff_t>(firstBody.GetUserdata()));
		Ndk::EntityId secondEntityId = static_cast<Ndk::EntityId>(reinterpret_cast<std::ptrdiff_t>(secondBody.GetUserdata()));

		const Ndk::EntityHandle& firstEntity = m_world.GetEntity(firstEntityId);
		const Ndk::EntityHandle& secondEntity = m_world.GetEntity(secondEntityId);

		Nz::Vector3f firstVel = firstBody.GetLinearVelocity();
		Nz::Vector3f secondVel = secondBody.GetLinearVelocity();

		float relativeForce = (firstVel - secondVel).GetLength();

		Nz::UInt16 damage = static_cast<Nz::UInt16>(relativeForce);

		if (firstEntity->HasComponent<HealthComponent>())
		{
			auto& health = firstEntity->GetComponent<HealthComponent>();
			health.Damage(damage, secondEntity);
		}

		if (secondEntity->HasComponent<HealthComponent>())
		{
			auto& health = secondEntity->GetComponent<HealthComponent>();
			health.Damage(damage, firstEntity);
		}

		return true;
	}

	bool Arena::HandlePlasmaProjectileCollision(const Nz::RigidBody3D& firstBody, const Nz::RigidBody3D& secondBody)
	{
		Ndk::EntityId laserEntityId = static_cast<Ndk::EntityId>(reinterpret_cast<std::ptrdiff_t>(firstBody.GetUserdata()));
		Ndk::EntityId hitEntityId = static_cast<Ndk::EntityId>(reinterpret_cast<std::ptrdiff_t>(secondBody.GetUserdata()));

		if (secondBody.GetMaterial() == m_plasmaMaterial)
		{
			assert(firstBody.GetMaterial() != m_plasmaMaterial);
			std::swap(laserEntityId, hitEntityId);
		}

		const Ndk::EntityHandle& projectile = m_world.GetEntity(laserEntityId);
		const Ndk::EntityHandle& hitEntity = m_world.GetEntity(hitEntityId);

		assert(projectile->HasComponent<ProjectileComponent>());

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

	bool Arena::HandleTorpedoProjectileCollision(const Nz::RigidBody3D& firstBody, const Nz::RigidBody3D& secondBody)
	{
		Ndk::EntityId torpedoEntityId = static_cast<Ndk::EntityId>(reinterpret_cast<std::ptrdiff_t>(firstBody.GetUserdata()));
		Ndk::EntityId hitEntityId = static_cast<Ndk::EntityId>(reinterpret_cast<std::ptrdiff_t>(secondBody.GetUserdata()));

		if (secondBody.GetMaterial() == m_torpedoMaterial)
		{
			assert(firstBody.GetMaterial() != m_plasmaMaterial);
			std::swap(torpedoEntityId, hitEntityId);
		}

		const Ndk::EntityHandle& projectile = m_world.GetEntity(torpedoEntityId);
		const Ndk::EntityHandle& hitEntity = m_world.GetEntity(hitEntityId);

		assert(projectile->HasComponent<ProjectileComponent>());

		ProjectileComponent& projectileComponent = projectile->GetComponent<ProjectileComponent>();
		if (projectileComponent.HasBeenHit(hitEntity))
			return false;

		projectileComponent.MarkAsHit(hitEntity);

		// Deal damage if entity has a health value

		// Apply physics force
		auto& projectilePhys = projectile->GetComponent<Ndk::PhysicsComponent3D>();

		Nz::PhysWorld3D& physWorld = m_world.GetSystem<Ndk::PhysicsSystem3D>().GetWorld();

		float explosionRadius = 50.f;
		Nz::Vector3f torpedoPosition = projectilePhys.GetPosition();
		Nz::Boxf detectionBox = Nz::Boxf(torpedoPosition - Nz::Vector3f(explosionRadius), torpedoPosition + Nz::Vector3f(explosionRadius));

		float maxSquaredRadius = explosionRadius * explosionRadius;
		physWorld.ForEachBodyInAABB(detectionBox, [&](Nz::RigidBody3D& body)
		{
			Nz::Vector3f bodyPosition = body.GetPosition();
			if (bodyPosition.SquaredDistance(torpedoPosition) < maxSquaredRadius)
			{
				Ndk::EntityId bodyId = static_cast<Ndk::EntityId>(reinterpret_cast<std::ptrdiff_t>(body.GetUserdata()));
				const Ndk::EntityHandle& bodyEntity = m_world.GetEntity(bodyId);

				float fade = std::clamp(bodyPosition.Distance(torpedoPosition) / explosionRadius, 0.f, 1.f);

				if (bodyEntity->HasComponent<HealthComponent>())
				{
					auto& health = bodyEntity->GetComponent<HealthComponent>();
					health.Damage(static_cast<Nz::UInt16>(projectileComponent.GetDamageValue() / fade), projectile);
				}

				Nz::Vector3f force = bodyPosition - torpedoPosition;
				force.Normalize();
				force *= 500'000.f / fade;

				body.AddForce(force);
			}

			return true;
		});

		projectile->Kill(); //< Remember entity destruction is not immediate, we can still use it safely

		return false;
	}

	void Arena::OnBroadcastEntitiesCreation(const BroadcastSystem* /*system*/, const Packets::CreateEntities& packet)
	{
		for (Player* player : m_players)
			player->SendPacket(packet);
	}

	void Arena::OnBroadcastEntitiesDestruction(const BroadcastSystem* /*system*/, const Packets::DeleteEntities& packet)
	{
		for (Player* player : m_players)
			player->SendPacket(packet);
	}

	void Arena::OnBroadcastStateUpdate(const BroadcastSystem* /*system*/, Packets::ArenaState& statePacket)
	{
		static Nz::UInt16 snapshotId = 0;
		statePacket.stateId = snapshotId++;

		for (Player* player : m_players)
		{
			statePacket.lastProcessedInputTime = player->GetLastInputProcessedTime();

			player->SendPacket(statePacket);
		}

		if constexpr (sendServerGhosts)
		{
			// Broadcast arena state over network, for testing purposes
			Nz::NetPacket debugState(1);
			PacketSerializer serializer(debugState, true);
			Packets::Serialize(serializer, statePacket);

			Nz::IpAddress debugAddress = Nz::IpAddress::BroadcastIpV4;
			debugAddress.SetPort(2050);

			m_debugSocket.SendPacket(debugAddress, debugState);
		}
	}
}
