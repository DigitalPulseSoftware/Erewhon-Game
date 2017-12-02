// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_ARENA_HPP
#define EREWHON_SERVER_ARENA_HPP

#include <Nazara/Core/Clock.hpp>
#include <NDK/EntityList.hpp>
#include <NDK/World.hpp>
#include <Shared/NetworkReactor.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Server/ServerCommandStore.hpp>
#include <unordered_set>
#include <vector>

namespace ewn
{
	class BroadcastSystem;
	class Player;
	class ServerApplication;

	class Arena
	{
		friend Player;

		public:
			Arena(ServerApplication* app);
			~Arena() = default;

			const Ndk::EntityHandle& CreatePlayerSpaceship(Player* owner);
			const Ndk::EntityHandle& CreateProjectile(Player* owner, const Nz::Vector3f& position);

			void DispatchChatMessage(Player* player, const Nz::String& message);

			void Update(float elapsedTime);

		private:
			const Ndk::EntityHandle& CreateEntity(std::string type, std::string name, const Nz::Vector3f& position);
			void HandlePlayerLeave(Player* player);
			void HandlePlayerJoin(Player* player);

			void OnBroadcastEntityCreation(const BroadcastSystem* system, const Packets::CreateEntity& packet);
			void OnBroadcastEntityDestruction(const BroadcastSystem* system, const Packets::DeleteEntity& packet);
			void OnBroadcastStateUpdate(const BroadcastSystem* system, Packets::ArenaState& statePacket);

			Nz::UdpSocket m_debugSocket;
			Ndk::EntityHandle m_attractionPoint;
			Ndk::World m_world;
			std::unordered_map<Player*, Ndk::EntityHandle> m_players;
			std::vector<Packets::CreateEntity> m_createEntityCache;
			ServerApplication* m_app;
			float m_stateBroadcastAccumulator;
	};
}

#include <Server/Arena.inl>

#endif // EREWHON_SERVER_ARENA_HPP
