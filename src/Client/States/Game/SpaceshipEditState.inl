// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/SpaceshipEditState.hpp>

namespace ewn
{
	inline SpaceshipEditState::SpaceshipEditState(StateData& stateData, std::shared_ptr<Ndk::State> previousState, std::string spaceshipName) :
	AbstractState(stateData),
	m_previousState(std::move(previousState)),
	m_spaceshipName(std::move(spaceshipName))
	{
	}
}
