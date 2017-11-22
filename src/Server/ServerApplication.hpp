// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_APPLICATION_HPP
#define EREWHON_SERVER_APPLICATION_HPP

#include <Shared/BaseApplication.hpp>
#include <Nazara/Core/MemoryPool.hpp>
#include <Server/Arena.hpp>
#include <Server/ServerCommandStore.hpp>
#include <vector>

namespace ewn
{
	class Player;

	class ServerApplication final : public BaseApplication
	{
		public:
			ServerApplication();
			virtual ~ServerApplication();

			bool Run() override;

			void HandleLogin(std::size_t peerId, const Packets::Login& data);
			void HandleJoinArena(std::size_t peerId, const Packets::JoinArena& data);
			void HandlePlayerChat(std::size_t peerId, const Packets::PlayerChat& data);
			void HandlePlayerMovement(std::size_t peerId, const Packets::PlayerMovement& data);
			void HandleTimeSyncRequest(std::size_t peerId, const Packets::TimeSyncRequest& data);

		private:
			void HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data) override;
			void HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data) override;
			void HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet) override;

			std::vector<Player*> m_players;
			Nz::MemoryPool m_playerPool;
			Arena m_arena;
			ServerCommandStore m_commandStore;
	};
}

#include <Server/ServerApplication.inl>

#endif // EREWHON_SERVER_APPLICATION_HPP
