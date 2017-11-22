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
	ClientApplication::ClientApplication() :
	m_commandStore(this),
	m_serverPeerId(NetworkReactor::InvalidPeerId),
	m_isServerConnected(false)
	{
	}

	ClientApplication::~ClientApplication() = default;

	bool ClientApplication::Connect(const Nz::String& serverHostname, Nz::UInt32 data)
	{
		if (m_serverPeerId != NetworkReactor::InvalidPeerId)
			GetReactor(0)->DisconnectPeer(m_serverPeerId, 0, DisconnectionType::Kick);

		Nz::ResolveError resolveError = Nz::ResolveError_NoError;
		std::vector<Nz::HostnameInfo> results = Nz::IpAddress::ResolveHostname(Nz::NetProtocol_IPv4, serverHostname, "2049", &resolveError);
		if (results.empty())
		{
			std::cerr << "Failed to resolve server hostname: " << Nz::ErrorToString(resolveError) << std::endl;
			return false;
		}

		m_serverPeerId = GetReactor(0)->ConnectTo(results.front().address, data);
		if (m_serverPeerId == NetworkReactor::InvalidPeerId)
		{
			std::cerr << "Failed to allocate new peer" << std::endl;
			return false;
		}

		return true;
	}

	bool ClientApplication::Run()
	{
		return BaseApplication::Run();
	}

	void ClientApplication::HandleArenaState(std::size_t peerId, const Packets::ArenaState& data)
	{
		assert(peerId == m_serverPeerId);

		OnArenaState(data);
	}

	void ClientApplication::HandleChatMessage(std::size_t peerId, const Packets::ChatMessage& data)
	{
		assert(peerId == m_serverPeerId);

		OnChatMessage(data);
	}

	void ClientApplication::HandleControlSpaceship(std::size_t peerId, const Packets::ControlSpaceship& data)
	{
		assert(peerId == m_serverPeerId);

		OnControlSpaceship(data);
	}

	void ClientApplication::HandleCreateSpaceship(std::size_t peerId, const Packets::CreateSpaceship& data)
	{
		assert(peerId == m_serverPeerId);

		OnCreateSpaceship(data);
	}

	void ClientApplication::HandleDeleteSpaceship(std::size_t peerId, const Packets::DeleteSpaceship& data)
	{
		assert(peerId == m_serverPeerId);

		OnDeleteSpaceship(data);
	}

	void ClientApplication::HandleLoginFailure(std::size_t peerId, const Packets::LoginFailure& data)
	{
		assert(peerId == m_serverPeerId);

		OnLoginFailed(data);
	}

	void ClientApplication::HandleLoginSuccess(std::size_t peerId, const Packets::LoginSuccess& data)
	{
		assert(peerId == m_serverPeerId);

		OnLoginSucceeded(data);
	}

	void ClientApplication::HandleTimeSyncResponse(std::size_t peerId, const Packets::TimeSyncResponse& data)
	{
		OnTimeSyncResponse(data);
	}

	void ClientApplication::HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data)
	{
		assert(peerId == m_serverPeerId);

		m_isServerConnected = true;

		OnServerConnected(data);
/*
		Packets::Login login;
		login.login = "Lynix";
		login.passwordHash = "FD47AC41DEADBEEF";

		Nz::NetPacket loginPacket;
		loginPacket << static_cast<Nz::UInt8>(PacketType::Login);
		Packets::Serialize(loginPacket, login);

		GetReactor(0)->SendData(m_serverPeerId, 0, Nz::ENetPacketFlag_Reliable, std::move(loginPacket));*/

		std::cout << "Connected to server with data " << data << std::endl;
	}

	void ClientApplication::HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data)
	{
		assert(peerId == m_serverPeerId);
		m_serverPeerId = NetworkReactor::InvalidPeerId;

		m_isServerConnected = false;

		OnServerDisconnected(data);

		std::cout << "Disconnected from server with data " << data << std::endl;
	}

	void ClientApplication::HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet)
	{
		assert(peerId == m_serverPeerId);

		m_commandStore.UnserializePacket(peerId, std::move(packet));

		//std::cout << "Receive packet from server of size " << packet.GetDataSize() << std::endl;
	}
}
