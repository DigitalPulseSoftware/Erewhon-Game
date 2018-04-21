// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/DisconnectionState.hpp>

namespace ewn
{
	inline DisconnectionState::DisconnectionState(StateData& stateData, bool quitApp) :
	AbstractState(stateData),
	m_shouldQuitApp(quitApp)
	{
	}
}
