// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/ArenaComponent.hpp>

namespace ewn
{
	inline ArenaComponent::ArenaComponent(Arena& arena) :
	m_arena(arena)
	{
	}

	inline Arena& ArenaComponent::GetArena()
	{
		return m_arena;
	}

	inline ArenaComponent::operator Arena&()
	{
		return m_arena;
	}
}
