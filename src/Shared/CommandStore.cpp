// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/CommandStore.hpp>
#include <iostream>

namespace ewn
{
	CommandStore::~CommandStore() = default;

	bool CommandStore::UnserializePacket(std::size_t peerId, Nz::NetPacket&& packet) const
	{
		Nz::UInt8 opcode;
		try
		{
			packet >> opcode;
		}
		catch (const std::exception&)
		{
			std::cerr << "Failed to unserialize opcode" << std::endl;
			return false;
		}

		if (m_incomingCommands.size() <= opcode || !m_incomingCommands[opcode].enabled)
		{
			std::cerr << "Client #" << peerId << " sent invalid opcode" << std::endl;
			return false;
		}

		m_incomingCommands[opcode].unserialize(peerId, std::move(packet));
		return true;
	}
}
