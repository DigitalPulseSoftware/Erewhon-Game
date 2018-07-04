// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_SERVERCONNECTION_HPP
#define EREWHON_CLIENT_SERVERCONNECTION_HPP

#include <Shared/Protocol/NetworkStringStore.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Client/ClientCommandStore.hpp>
#include <Nazara/Core/Signal.hpp>
#include <Nazara/Core/String.hpp>

namespace ewn
{
	class ClientApplication;
	class ClientCommandStore;
	class NetworkReactor;

	class ServerConnection
	{
		friend ClientApplication;

		public:
			struct ConnectionInfo;

			inline ServerConnection(ClientApplication& application);
			ServerConnection(const ServerConnection&) = delete;
			ServerConnection(ServerConnection&&) = delete;
			~ServerConnection() = default;

			bool Connect(const Nz::String& serverHostname, Nz::UInt32 data = 0);
			inline void Disconnect(Nz::UInt32 data = 0);

			Nz::UInt64 EstimateServerTime() const;

			inline ClientApplication& GetApp();
			inline const ClientApplication& GetApp() const;
			inline const ConnectionInfo& GetConnectionInfo() const;
			inline const NetworkStringStore& GetNetworkStringStore() const;
			inline std::size_t GetPeerId() const;

			inline bool IsConnected() const;

			inline void RefreshInfos();

			template<typename T> void SendPacket(const T& packet);

			inline void UpdateServerTimeDelta(Nz::UInt64 deltaTime);

			ServerConnection& operator=(const ServerConnection&) = delete;
			ServerConnection& operator=(ServerConnection&&) = delete;

			NazaraSignal(OnConnected,            ServerConnection* /*server*/, Nz::UInt32 /*data*/);
			NazaraSignal(OnConnectionInfoUpdate, ServerConnection* /*server*/, const ConnectionInfo& /*info*/);
			NazaraSignal(OnDisconnected,         ServerConnection* /*server*/, Nz::UInt32 /*data*/);

			// Packet reception signals
			NazaraSignal(OnArenaList,                 ServerConnection* /*server*/, const Packets::ArenaList&                 /*data*/);
			NazaraSignal(OnArenaPrefabs,              ServerConnection* /*server*/, const Packets::ArenaPrefabs&              /*data*/);
			NazaraSignal(OnArenaParticleSystems,      ServerConnection* /*server*/, const Packets::ArenaParticleSystems&      /*data*/);
			NazaraSignal(OnArenaSounds,               ServerConnection* /*server*/, const Packets::ArenaSounds&               /*data*/);
			NazaraSignal(OnArenaState,                ServerConnection* /*server*/, const Packets::ArenaState&                /*data*/);
			NazaraSignal(OnBotMessage,                ServerConnection* /*server*/, const Packets::BotMessage&                /*data*/);
			NazaraSignal(OnChatMessage,               ServerConnection* /*server*/, const Packets::ChatMessage&               /*data*/);
			NazaraSignal(OnControlEntity,             ServerConnection* /*server*/, const Packets::ControlEntity&             /*data*/);
			NazaraSignal(OnCreateEntity,              ServerConnection* /*server*/, const Packets::CreateEntity&              /*data*/);
			NazaraSignal(OnCreateFleetFailure,        ServerConnection* /*server*/, const Packets::CreateFleetFailure&        /*data*/);
			NazaraSignal(OnCreateFleetSuccess,        ServerConnection* /*server*/, const Packets::CreateFleetSuccess&        /*data*/);
			NazaraSignal(OnCreateSpaceshipFailure,    ServerConnection* /*server*/, const Packets::CreateSpaceshipFailure&    /*data*/);
			NazaraSignal(OnCreateSpaceshipSuccess,    ServerConnection* /*server*/, const Packets::CreateSpaceshipSuccess&    /*data*/);
			NazaraSignal(OnDeleteEntity,              ServerConnection* /*server*/, const Packets::DeleteEntity&              /*data*/);
			NazaraSignal(OnDeleteFleetFailure,        ServerConnection* /*server*/, const Packets::DeleteFleetFailure&        /*data*/);
			NazaraSignal(OnDeleteFleetSuccess,        ServerConnection* /*server*/, const Packets::DeleteFleetSuccess&        /*data*/);
			NazaraSignal(OnDeleteSpaceshipFailure,    ServerConnection* /*server*/, const Packets::DeleteSpaceshipFailure&    /*data*/);
			NazaraSignal(OnDeleteSpaceshipSuccess,    ServerConnection* /*server*/, const Packets::DeleteSpaceshipSuccess&    /*data*/);
			NazaraSignal(OnFleetInfo,                 ServerConnection* /*server*/, const Packets::FleetInfo&                 /*data*/);
			NazaraSignal(OnFleetList,                 ServerConnection* /*server*/, const Packets::FleetList&                 /*data*/);
			NazaraSignal(OnHullList,                  ServerConnection* /*server*/, const Packets::HullList&                  /*data*/);
			NazaraSignal(OnInstantiateParticleSystem, ServerConnection* /*server*/, const Packets::InstantiateParticleSystem& /*data*/);
			NazaraSignal(OnIntegrityUpdate,           ServerConnection* /*server*/, const Packets::IntegrityUpdate&           /*data*/);
			NazaraSignal(OnLoginFailure,              ServerConnection* /*server*/, const Packets::LoginFailure&              /*data*/);
			NazaraSignal(OnLoginSuccess,              ServerConnection* /*server*/, const Packets::LoginSuccess&              /*data*/);
			NazaraSignal(OnModuleList,                ServerConnection* /*server*/, const Packets::ModuleList&                /*data*/);
			NazaraSignal(OnNetworkStrings,            ServerConnection* /*server*/, const Packets::NetworkStrings&            /*data*/);
			NazaraSignal(OnPlaySound,                 ServerConnection* /*server*/, const Packets::PlaySound&                 /*data*/);
			NazaraSignal(OnRegisterFailure,           ServerConnection* /*server*/, const Packets::RegisterFailure&           /*data*/);
			NazaraSignal(OnRegisterSuccess,           ServerConnection* /*server*/, const Packets::RegisterSuccess&           /*data*/);
			NazaraSignal(OnSpaceshipInfo,             ServerConnection* /*server*/, const Packets::SpaceshipInfo&             /*data*/);
			NazaraSignal(OnSpaceshipList,             ServerConnection* /*server*/, const Packets::SpaceshipList&             /*data*/);
			NazaraSignal(OnTimeSyncResponse,          ServerConnection* /*server*/, const Packets::TimeSyncResponse&          /*data*/);
			NazaraSignal(OnUpdateFleetFailure,        ServerConnection* /*server*/, const Packets::UpdateFleetFailure&        /*data*/);
			NazaraSignal(OnUpdateFleetSuccess,        ServerConnection* /*server*/, const Packets::UpdateFleetSuccess&        /*data*/);
			NazaraSignal(OnUpdateSpaceshipFailure,    ServerConnection* /*server*/, const Packets::UpdateSpaceshipFailure&    /*data*/);
			NazaraSignal(OnUpdateSpaceshipSuccess,    ServerConnection* /*server*/, const Packets::UpdateSpaceshipSuccess&    /*data*/);

			struct ConnectionInfo
			{
				Nz::UInt32 ping;
				Nz::UInt64 lastReceiveTime;
			};

		private:
			inline void DispatchIncomingPacket(Nz::NetPacket&& packet);
			inline void NotifyConnected(Nz::UInt32 data);
			inline void NotifyDisconnected(Nz::UInt32 data);
			inline void UpdateInfo(const ConnectionInfo& connectionInfo);

			void UpdateNetworkStrings(ServerConnection* server, const Packets::NetworkStrings& data);

			ClientApplication& m_application;
			ClientCommandStore m_commandStore;
			NetworkStringStore m_stringStore;
			NetworkReactor* m_networkReactor;
			ConnectionInfo m_connectionInfo;
			Nz::UInt64 m_deltaTime;
			std::size_t m_peerId;
			bool m_connected;
	};
}

#include <Client/ServerConnection.inl>

#endif // EREWHON_CLIENT_SERVERCONNECTION_HPP
