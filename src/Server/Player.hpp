// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_PLAYER_HPP
#define EREWHON_SERVER_PLAYER_HPP

#include <Nazara/Core/HandledObject.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <NDK/EntityOwner.hpp>
#include <Shared/NetworkReactor.hpp>
#include <Server/ServerCommandStore.hpp>

namespace ewn
{
	class Arena;

	class Player;

	using PlayerHandle = Nz::ObjectHandle<Player>;

	class Player : public Nz::HandledObject<Player>
	{
		friend class ServerCommandStore;

		public:
			Player(ServerApplication* app, std::size_t peerId, NetworkReactor& reactor, const ServerCommandStore& commandStore);
			~Player();

			void Authenticate(std::string login);

			inline void Disconnect(Nz::UInt32 data = 0);

			inline Arena* GetArena() const;
			const Ndk::EntityHandle& GetBotEntity();
			Nz::UInt64 GetLastInputProcessedTime() const;
			inline const std::string& GetName() const;
			inline std::size_t GetPeerId() const;

			inline bool IsAuthenticated() const;

			void MoveToArena(Arena* arena);

			template<typename T> void SendPacket(const T& packet);

			void Shoot();

			void UpdateInput(Nz::UInt64 time, Nz::Vector3f direction, Nz::Vector3f rotation);

		private:
			Arena* m_arena;
			ServerApplication* m_app;
			NetworkReactor& m_networkReactor;
			const ServerCommandStore& m_commandStore;
			std::size_t m_peerId;
			std::string m_login;
			Ndk::EntityOwner m_botEntity;
			Ndk::EntityOwner m_spaceship;
			Nz::UInt64 m_lastInputTime;
			Nz::UInt64 m_lastShootTime;
			bool m_authenticated;
	};
}

#include <Server/Player.inl>

#endif // EREWHON_SERVER_PLAYER_HPP
