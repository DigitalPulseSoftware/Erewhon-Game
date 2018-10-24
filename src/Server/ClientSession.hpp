// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_CLIENTSESSION_HPP
#define EREWHON_SERVER_CLIENTSESSION_HPP

#include <Shared/NetworkReactor.hpp>
#include <Server/ServerCommandStore.hpp>

namespace ewn
{
	class Player;
	class ServerApplication;

	class ClientSession
	{
		friend class ServerApplication;
		friend class ServerCommandStore;

		public:
			ClientSession(ServerApplication* app, std::size_t sessionId, std::size_t peerId, std::shared_ptr<Player> player, NetworkReactor& reactor, const ServerCommandStore& commandStore);
			~ClientSession() = default;

			inline void Disconnect(Nz::UInt32 data = 0);

			inline std::size_t GetPeerId() const;
			inline Player* GetPlayer();
			inline const Player* GetPlayer() const;
			inline std::size_t GetSessionId() const;

			template<typename T> void SendPacket(const T& packet);

		private:
			void HandleControlEntity(const Packets::ControlEntity& data);
			void HandleCreateFleet(const Packets::CreateFleet& data);
			void HandleCreateSpaceship(const Packets::CreateSpaceship& data);
			void HandleDeleteFleet(const Packets::DeleteFleet& data);
			void HandleDeleteSpaceship(const Packets::DeleteSpaceship& data);
			void HandleLogin(const Packets::Login& data);
			void HandleLoginByToken(const Packets::LoginByToken& data);
			void HandleLoginSucceeded(Nz::Int32 databaseId, bool regenerateToken);
			void HandleLeaveArena(const Packets::LeaveArena& data);
			void HandleJoinArena(const Packets::JoinArena& data);
			void HandlePlayerChat(const Packets::PlayerChat& data);
			void HandlePlayerMovement(const Packets::PlayerMovement& data);
			void HandlePlayerShoot(const Packets::PlayerShoot& data);
			void HandleQueryArenaList(const Packets::QueryArenaList& data);
			void HandleQueryFleetInfo(const Packets::QueryFleetInfo& data);
			void HandleQueryFleetList(const Packets::QueryFleetList& data);
			void HandleQueryHullList(const Packets::QueryHullList& data);
			void HandleQueryModuleList(const Packets::QueryModuleList& data);
			void HandleQuerySpaceshipInfo(const Packets::QuerySpaceshipInfo& data);
			void HandleQuerySpaceshipList(const Packets::QuerySpaceshipList& data);
			void HandleRegister(const Packets::Register& data);
			void HandleTimeSyncRequest(const Packets::TimeSyncRequest& data);
			void HandleUpdateFleet(const Packets::UpdateFleet& data);
			void HandleUpdateSpaceship(const Packets::UpdateSpaceship& data);

			std::shared_ptr<Player> m_player;
			std::size_t m_peerId;
			std::size_t m_sessionId;
			ServerApplication* m_app;
			NetworkReactor& m_networkReactor;
			const ServerCommandStore& m_commandStore;
	};
}

#include <Server/ClientSession.inl>

#endif // EREWHON_SERVER_CLIENTSESSION_HPP
