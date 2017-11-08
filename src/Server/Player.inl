// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Player.hpp>
#include <Nazara/Network/NetPacket.hpp>
#include <Shared/Protocol/Packets.hpp>

namespace ewn
{
	template<typename T>
	void Player::SendPacket(const T& packet)
	{
		const auto& command = m_commandStore.GetOutgoingCommand<T>();
		
		Nz::NetPacket data;
		m_commandStore.SerializePacket(data, packet);

		m_networkReactor.SendData(m_peerId, command.channelId, command.flags, std::move(data));
	}
}