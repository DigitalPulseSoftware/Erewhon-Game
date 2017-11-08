// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_PLAYER_HPP
#define EREWHON_SERVER_PLAYER_HPP

#include <Shared/NetworkReactor.hpp>
#include <Server/ServerCommandStore.hpp>

namespace ewn
{
	class Player
	{
		friend class ServerCommandStore;

		public:
			Player(std::size_t peerId, NetworkReactor& reactor, const ServerCommandStore& commandStore);
			~Player() = default;

			template<typename T> void SendPacket(const T& packet);

		private:
			static void HandleLogin(std::size_t peerId, const Packets::Login& data);

			const ServerCommandStore& m_commandStore;
			NetworkReactor& m_networkReactor;
			std::size_t m_peerId;
	};
}

#include <Server/Player.inl>

#endif // EREWHON_SERVER_PLAYER_HPP