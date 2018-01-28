// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientApplication.hpp>
#include <Nazara/Network/Algorithm.hpp>
#include <Shared/Config.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <iostream>

namespace ewn
{
	ClientApplication::ClientApplication()
	{
		m_config.RegisterStringOption("ClientScript.Filename");
		m_config.RegisterStringOption("ServerScript.Filename");

		m_config.RegisterStringOption("Server.Address");
		m_config.RegisterIntegerOption("Server.Port", 1, 0xFFFF);
	}

	ClientApplication::~ClientApplication() = default;

	bool ClientApplication::Run()
	{
		return BaseApplication::Run();
	}

	bool ClientApplication::ConnectNewServer(const Nz::String& serverHostname, Nz::UInt32 data, ServerConnection* connection, std::size_t* peerId, NetworkReactor** reactor)
	{
		long long port = m_config.GetIntegerOption("Server.Port");

		Nz::ResolveError resolveError = Nz::ResolveError_NoError;
		std::vector<Nz::HostnameInfo> results = Nz::IpAddress::ResolveHostname(Nz::NetProtocol_IPv4, serverHostname, Nz::String::Number(port), &resolveError);
		if (results.empty())
		{
			std::cerr << "Failed to resolve server hostname: " << Nz::ErrorToString(resolveError) << std::endl;
			return false;
		}

		std::size_t newPeerId = GetReactor(0)->ConnectTo(results.front().address, data);
		if (newPeerId == NetworkReactor::InvalidPeerId)
		{
			std::cerr << "Failed to allocate new peer" << std::endl;
			return false;
		}

		*peerId = newPeerId;
		*reactor = GetReactor(0).get();

		if (newPeerId >= m_servers.size())
			m_servers.resize(newPeerId + 1);

		m_servers[newPeerId] = connection;

		return true;
	}

	void ClientApplication::HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data)
	{
		m_servers[peerId]->NotifyConnected(data);
	}

	void ClientApplication::HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data)
	{
		m_servers[peerId]->NotifyDisconnected(data);
		m_servers[peerId] = nullptr;
	}

	void ClientApplication::HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet)
	{
		m_servers[peerId]->DispatchIncomingPacket(std::move(packet));
	}
}
