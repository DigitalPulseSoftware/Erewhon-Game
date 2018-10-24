// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
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
		RegisterConfig();
	}

	ClientApplication::~ClientApplication() = default;

	bool ClientApplication::Run()
	{
		return BaseApplication::Run();
	}

	bool ClientApplication::ConnectNewServer(const Nz::String& serverHostname, Nz::UInt32 data, ServerConnection* connection, std::size_t* peerId, NetworkReactor** peerReactor)
	{
		constexpr std::size_t MaxPeerCount = 1;

		Nz::UInt16 port = m_config.GetIntegerOption<Nz::UInt16>("Server.Port");

		Nz::NetProtocol hostnameProtocol = (m_config.GetBoolOption("Options.ForceIPv4")) ? Nz::NetProtocol_IPv4 : Nz::NetProtocol_Any;

		Nz::ResolveError resolveError = Nz::ResolveError_NoError;
		std::vector<Nz::HostnameInfo> results = Nz::IpAddress::ResolveHostname(hostnameProtocol, serverHostname, Nz::String::Number(port), &resolveError);
		if (results.empty())
		{
			std::cerr << "Failed to resolve server hostname: " << Nz::ErrorToString(resolveError) << std::endl;
			return false;
		}

		Nz::IpAddress serverAddress = results.front().address;

		auto ConnectWithReactor = [&](NetworkReactor* reactor) -> bool
		{
			std::size_t newPeerId = reactor->ConnectTo(serverAddress, data);
			if (newPeerId == NetworkReactor::InvalidPeerId)
			{
				std::cerr << "Failed to allocate new peer" << std::endl;
				return false;
			}

			*peerId = newPeerId;
			*peerReactor = reactor;

			if (newPeerId >= m_servers.size())
				m_servers.resize(newPeerId + 1);

			m_servers[newPeerId] = connection;
			return true;
		};

		std::size_t reactorCount = GetReactorCount();
		std::size_t reactorIndex;
		for (reactorIndex = 0; reactorIndex < reactorCount; ++reactorIndex)
		{
			const std::unique_ptr<NetworkReactor>& reactor = GetReactor(reactorIndex);
			if (reactor->GetProtocol() != serverAddress.GetProtocol())
				continue;

			return ConnectWithReactor(reactor.get());
		}

		// We don't have any reactor compatible with the server's protocol, allocate a new one
		std::size_t reactorId = AddReactor(std::make_unique<NetworkReactor>(reactorCount * MaxPeerCount, serverAddress.GetProtocol(), 0, MaxPeerCount));
		return ConnectWithReactor(GetReactor(reactorId).get());
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

	void ClientApplication::HandlePeerInfo(std::size_t peerId, const NetworkReactor::PeerInfo& peerInfo)
	{
		ServerConnection::ConnectionInfo connectionInfo;
		connectionInfo.lastReceiveTime = GetAppTime() - peerInfo.lastReceiveTime;
		connectionInfo.ping = peerInfo.ping;

		m_servers[peerId]->UpdateInfo(connectionInfo);
	}

	void ClientApplication::HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet)
	{
		m_servers[peerId]->DispatchIncomingPacket(std::move(packet));
	}

	void ClientApplication::RegisterConfig()
	{
		m_config.RegisterStringOption("AssetsFolder");

		m_config.RegisterStringOption("ClientScript.Filename");
		m_config.RegisterStringOption("ServerScript.Filename");

		m_config.RegisterBoolOption("Options.Fullscreen");
		m_config.RegisterBoolOption("Options.ForceIPv4");
		m_config.RegisterBoolOption("Options.VerticalSync");

		m_config.RegisterIntegerOption("Security.Argon2.IterationCost");
		m_config.RegisterIntegerOption("Security.Argon2.MemoryCost");
		m_config.RegisterIntegerOption("Security.Argon2.ThreadCost");
		m_config.RegisterIntegerOption("Security.HashLength");
		m_config.RegisterStringOption("Security.PasswordSalt");

		m_config.RegisterStringOption("Server.Address");
		m_config.RegisterIntegerOption("Server.Port", 1, 0xFFFF);
	}
}
