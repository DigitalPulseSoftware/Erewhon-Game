// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_APPLICATION_HPP
#define EREWHON_CLIENT_APPLICATION_HPP

#include <Nazara/Core/Signal.hpp>
#include <Nazara/Network/ENetHost.hpp>
#include <Shared/BaseApplication.hpp>
#include <Shared/NetworkReactor.hpp>
#include <Client/ClientCommandStore.hpp>
#include <Client/ServerConnection.hpp>
#include <memory>
#include <vector>

namespace ewn
{
	class ClientApplication final : public BaseApplication
	{
		friend class ServerConnection;

		public:
			ClientApplication();

			virtual ~ClientApplication();

			bool Run() override;

		private:
			bool ConnectNewServer(const Nz::String& serverHostname, Nz::UInt32 data, ServerConnection* connection, std::size_t* peerId, NetworkReactor** peerReactor);

			void HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data) override;
			void HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data) override;
			void HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet) override;

			void RegisterConfig();

			std::vector<ServerConnection*> m_servers;
	};
}

#include <Client/ClientApplication.inl>

#endif // EREWHON_CLIENT_APPLICATION_HPP
