// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_ARENA_HPP
#define EREWHON_SERVER_ARENA_HPP

#include <Nazara/Core/Clock.hpp>
#include <NDK/EntityList.hpp>
#include <NDK/EntityOwner.hpp>
#include <NDK/World.hpp>
#include <Shared/NetworkReactor.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Server/ServerCommandStore.hpp>
#include <unordered_set>
#include <vector>

namespace Nz
{
	class RigidBody3D;
}

namespace ewn
{
	class BroadcastSystem;
	class Player;
	class ServerApplication;

	class Arena
	{
		friend Player;

		public:
			Arena(ServerApplication* app, std::string name);
			Arena(const Arena&) = delete;
			Arena(Arena&&) = delete;
			~Arena();

			template<typename T>
			void BroadcastPacket(const T& packet, Player* exceptPlayer = nullptr);

			const Ndk::EntityHandle& CreatePlayerSpaceship(Player* owner);
			const Ndk::EntityHandle& CreatePlasmaProjectile(Player* owner, const Ndk::EntityHandle& emitter, const Nz::Vector3f& position, const Nz::Quaternionf& rotation);
			const Ndk::EntityHandle& CreateTorpedo(Player* owner, const Ndk::EntityHandle& emitter, const Nz::Vector3f& position, const Nz::Quaternionf& rotation);

			void DispatchChatMessage(const Nz::String& message);

			Player* FindPlayerByName(const std::string& name) const;

			inline const std::string& GetName() const;

			void Reset();

			void SpawnFleet(Player* owner, const std::string& fleetName);
			void SpawnSpaceship(Player* owner, const std::string& spaceshipName, const Nz::Vector3f& position, const Nz::Quaternionf& rotation);
			void SpawnSpaceship(Player* owner, Nz::Int32 spaceshipId, const Nz::Vector3f& position, const Nz::Quaternionf& rotation);

			void Update(float elapsedTime);

			Arena& operator=(const Arena&) = delete;
			Arena& operator=(Arena&&) = delete;

		private:
			const Ndk::EntityHandle& CreateEntity(std::string type, std::string name, Player* owner, const Nz::Vector3f& position, const Nz::Quaternionf& rotation);
			const Ndk::EntityHandle& CreateSpaceship(std::string name, Player* owner, std::size_t spaceshipHullId, const Nz::Vector3f& position, const Nz::Quaternionf& rotation);
			void HandlePlayerLeave(Player* player);
			void HandlePlayerJoin(Player* player);

			bool HandlePlasmaProjectileCollision(const Nz::RigidBody3D& firstBody, const Nz::RigidBody3D& secondBody);
			bool HandleTorpedoProjectileCollision(const Nz::RigidBody3D& firstBody, const Nz::RigidBody3D& secondBody);

			void OnBroadcastEntityCreation(const BroadcastSystem* system, const Packets::CreateEntity& packet);
			void OnBroadcastEntityDestruction(const BroadcastSystem* system, const Packets::DeleteEntity& packet);
			void OnBroadcastStateUpdate(const BroadcastSystem* system, Packets::ArenaState& statePacket);

			void SendArenaData(Player* player);

			void SpawnSpaceship(Player* owner, Nz::Int32 spaceshipId, std::string code, std::size_t spaceshipHullId, const Nz::Vector3f& position, const Nz::Quaternionf& rotation);

			struct PlayerData
			{
				Nz::UInt64 deathTime = 0;
			};

			Nz::UdpSocket m_debugSocket;
			Ndk::EntityOwner m_earth;
			Ndk::EntityOwner m_light;
			Ndk::EntityOwner m_spaceball;
			Ndk::EntityList m_scriptControlledEntities;
			Ndk::World m_world;
			std::string m_name;
			std::unordered_map<Player*, PlayerData> m_players;
			std::vector<Packets::CreateEntity> m_createEntityCache;
			ServerApplication* m_app;
			float m_stateBroadcastAccumulator;
			int m_plasmaMaterial;
			int m_torpedoMaterial;
	};
}

#include <Server/Arena.inl>

#endif // EREWHON_SERVER_ARENA_HPP
