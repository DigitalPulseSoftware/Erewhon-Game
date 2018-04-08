// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Arena.hpp>
#include <Server/Player.hpp>

namespace ewn
{
	template<typename T>
	void Arena::BroadcastPacket(const T& packet, Player* exceptPlayer)
	{
		for (const auto& pair : m_players)
		{
			if (pair.first != exceptPlayer)
				pair.first->SendPacket(packet);
		}
	}
}
