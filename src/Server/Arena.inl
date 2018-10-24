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
		for (Player* player : m_players)
		{
			if (player != exceptPlayer)
				player->SendPacket(packet);
		}
	}

	inline const Ndk::EntityHandle& Arena::GetEntity(Ndk::EntityId entityId)
	{
		assert(IsEntityIdValid(entityId));
		return m_world.GetEntity(entityId);
	}

	inline Nz::LuaInstance& Arena::GetLuaInstance()
	{
		return m_script;
	}

	inline const std::string& Arena::GetName() const
	{
		return m_name;
	}

	inline bool Arena::IsEntityIdValid(Ndk::EntityId entityId) const
	{
		return m_world.IsEntityIdValid(entityId);
	}
}
