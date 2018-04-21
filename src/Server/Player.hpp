// Copyright (C) 2018 Jérôme Leclercq
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
			Player(ServerApplication* app, std::size_t peerId, std::size_t sessionId, NetworkReactor& reactor, const ServerCommandStore& commandStore);
			~Player();

			void Authenticate(Nz::Int32 dbId, std::function<void (Player*, bool succeeded)> authenticationCallback);

			inline void Disconnect(Nz::UInt32 data = 0);

			inline Arena* GetArena() const;
			inline const Ndk::EntityHandle& GetBotEntity() const;
			inline const Ndk::EntityHandle& GetControlledEntity() const;
			inline Nz::Int32 GetDatabaseId() const;
			Nz::UInt64 GetLastInputProcessedTime() const;
			inline const std::string& GetLogin() const;
			inline Nz::UInt16 GetPermissionLevel() const;
			inline const std::string& GetName() const;
			inline std::size_t GetPeerId() const;
			inline std::size_t GetSessionId() const;

			const Ndk::EntityHandle& InstantiateBot(std::size_t spaceshipHullId);

			inline bool IsAuthenticated() const;

			void MoveToArena(Arena* arena);

			void PrintMessage(std::string chatMessage);

			template<typename T> void SendPacket(const T& packet);

			void Shoot();

			void UpdateControlledEntity(const Ndk::EntityHandle& entity);
			void UpdateInput(Nz::UInt64 time, Nz::Vector3f direction, Nz::Vector3f rotation);
			void UpdatePermissionLevel(Nz::UInt16 permissionLevel, std::function<void(bool updateSucceeded)> databaseCallback = nullptr);

		private:
			void OnAuthenticated(std::string login, std::string displayName, Nz::UInt16 permissionLevel);

			Arena* m_arena;
			ServerApplication* m_app;
			NetworkReactor& m_networkReactor;
			const ServerCommandStore& m_commandStore;
			std::size_t m_peerId;
			std::size_t m_sessionId;
			std::string m_displayName;
			std::string m_login;
			Ndk::EntityOwner m_botEntity;
			Ndk::EntityOwner m_controlledEntity;
			Nz::Int32 m_databaseId;
			Nz::UInt16 m_permissionLevel;
			Nz::UInt64 m_lastInputTime;
			Nz::UInt64 m_lastShootTime;
			bool m_authenticated;
	};
}

#include <Server/Player.inl>

#endif // EREWHON_SERVER_PLAYER_HPP
