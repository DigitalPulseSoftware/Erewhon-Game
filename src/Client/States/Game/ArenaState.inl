// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/ArenaState.hpp>

namespace ewn
{
	inline ArenaState::ArenaState(StateData& stateData, Nz::UInt8 arenaIndex) :
	AbstractState(stateData),
	m_arenaIndex(arenaIndex)
	{
	}
}
