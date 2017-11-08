// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientApplication.hpp>

namespace ewn
{
	inline bool ClientApplication::IsConnected() const
	{
		return m_isServerConnected;
	}

	template<typename T>
	void ClientApplication::SendPacket(const T& packet)
	{
		if (m_serverPeerId == NetworkReactor::InvalidPeerId)
			return;

		const auto& command = m_commandStore.GetOutgoingCommand<T>();

		Nz::NetPacket data;
		m_commandStore.SerializePacket(data, packet);

		GetReactor(0)->SendData(m_serverPeerId, command.channelId, command.flags, std::move(data));
	}
}
