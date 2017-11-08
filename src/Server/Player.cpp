// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Player.hpp>
#include <iostream>

namespace ewn
{
	Player::Player(std::size_t peerId, NetworkReactor& reactor, const ServerCommandStore& commandStore) :
	m_commandStore(commandStore),
	m_networkReactor(reactor),
	m_peerId(peerId)
	{
	}
}
