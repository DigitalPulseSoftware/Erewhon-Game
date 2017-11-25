// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
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
	}

	inline void ServerConnection::Disconnect(Nz::UInt32 data)
	{
		m_networkReactor->DisconnectPeer(m_peerId, data);
	}

	inline bool ServerConnection::IsConnected()
	{
		return m_connected;
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
		m_peerId = NetworkReactor::InvalidPeerId;
		m_connected = false;

		OnDisconnected(this, data);
	}
}
