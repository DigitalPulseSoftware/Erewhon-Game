// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/TimeSyncState.hpp>

namespace ewn
{
	inline TimeSyncState::TimeSyncState(StateData& stateData, Nz::UInt8 arenaIndex) :
	AbstractState(stateData),
	m_arenaIndex(arenaIndex)
	{
	}
}
