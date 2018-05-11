// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ServerConnection.hpp>
#include <Nazara/Network/NetPacket.hpp>
#include <Client/ClientCommandStore.hpp>
#include <Shared/NetworkReactor.hpp>

namespace ewn
{
	inline ServerConnection::ServerConnection(ClientApplication& application) :
	m_application(application),
	m_commandStore(this),
	m_networkReactor(nullptr),
	m_peerId(NetworkReactor::InvalidPeerId),
	m_connected(false)
	{
		OnNetworkStrings.Connect([this](ServerConnection* server, const Packets::NetworkStrings& data) { UpdateNetworkStrings(server, data); });
	}

	inline void ServerConnection::Disconnect(Nz::UInt32 data)
	{
		m_networkReactor->DisconnectPeer(m_peerId, data);
	}

	inline ClientApplication& ServerConnection::GetApp()
	{
		return m_application;
	}

	inline const ClientApplication& ServerConnection::GetApp() const
	{
		return m_application;
	}

	inline const NetworkStringStore& ServerConnection::GetNetworkStringStore() const
	{
		return m_stringStore;
	}

	inline bool ServerConnection::IsConnected() const
	{
		return m_connected;
	}

	inline void ServerConnection::UpdateServerTimeDelta(Nz::UInt64 deltaTime)
	{
		m_deltaTime = deltaTime;
	}

	template<typename T>
	void ServerConnection::SendPacket(const T& packet)
	{
		if (!IsConnected())
			return;

		const auto& command = m_commandStore.GetOutgoingCommand<T>();

		Nz::NetPacket data;
		m_commandStore.SerializePacket(data, packet);

		m_networkReactor->SendData(m_peerId, command.channelId, command.flags, std::move(data));
	}

	inline void ServerConnection::DispatchIncomingPacket(Nz::NetPacket&& packet)
	{
		m_commandStore.UnserializePacket(m_peerId, std::move(packet));
	}

	inline void ServerConnection::NotifyConnected(Nz::UInt32 data)
	{
		m_connected = true;

		OnConnected(this, data);
	}

	inline void ServerConnection::NotifyDisconnected(Nz::UInt32 data)
	{
		m_connected = false;
		m_peerId = NetworkReactor::InvalidPeerId;
		m_stringStore.Clear();

		OnDisconnected(this, data);
	}
}
