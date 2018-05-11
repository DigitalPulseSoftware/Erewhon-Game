// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/MainMenuState.hpp>

namespace ewn
{
	inline MainMenuState::MainMenuState(StateData& stateData, std::string playerName) :
	AbstractState(stateData),
	m_playerName(playerName)
	{
	}
}
