// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_PLAYER_HPP
#define EREWHON_SERVER_PLAYER_HPP

#include <NDK/EntityOwner.hpp>
#include <Shared/NetworkReactor.hpp>
#include <Server/ServerCommandStore.hpp>

namespace ewn
{
	class Arena;

	class Player
	{
		friend class ServerCommandStore;

		public:
			Player(std::size_t peerId, NetworkReactor& reactor, const ServerCommandStore& commandStore);
			~Player();

			void Authenticate(std::string login);

			inline void Disconnect(Nz::UInt32 data = 0);

			inline Arena* GetArena() const;

			inline bool IsAuthenticated() const;

			void MoveToArena(Arena* arena);

			template<typename T> void SendPacket(const T& packet);

			void UpdateInput(const Nz::Vector3f& direction, const Nz::Vector3f& rotation);

		private:
			const ServerCommandStore& m_commandStore;
			Arena* m_arena;
			NetworkReactor& m_networkReactor;
			std::size_t m_peerId;
			std::string m_login;
			Ndk::EntityOwner m_spaceship;
			bool m_authenticated;
	};
}

#include <Server/Player.inl>

#endif // EREWHON_SERVER_PLAYER_HPP
