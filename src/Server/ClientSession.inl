// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ClientSession.hpp>
#include <Nazara/Network/NetPacket.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <cassert>

namespace ewn
{
	inline void ClientSession::Disconnect(Nz::UInt32 data)
	{
		m_networkReactor.DisconnectPeer(m_peerId, data);
	}

	inline std::size_t ClientSession::GetPeerId() const
	{
		return m_peerId;
	}

	inline Player* ClientSession::GetPlayer()
	{
		return m_player.get();
	}

	inline const Player* ClientSession::GetPlayer() const
	{
		return m_player.get();
	}

	inline std::size_t ClientSession::GetSessionId() const
	{
		return m_sessionId;
	}

	template<typename T>
	void ClientSession::SendPacket(const T& packet)
	{
		const auto& command = m_commandStore.GetOutgoingCommand<T>();
		
		Nz::NetPacket data;
		m_commandStore.SerializePacket(data, packet);

		m_networkReactor.SendData(m_peerId, command.channelId, command.flags, std::move(data));
	}
}
