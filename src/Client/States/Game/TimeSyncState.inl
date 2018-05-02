// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/TimeSyncState.hpp>

namespace ewn
{
	inline TimeSyncState::TimeSyncState(StateData& stateData, std::string playerName) :
	AbstractState(stateData),
	m_playerName(playerName)
	{
	}
}
