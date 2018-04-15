// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/SpaceshipListState.hpp>

namespace ewn
{
	inline SpaceshipListState::SpaceshipListState(StateData& stateData, std::shared_ptr<Ndk::State> previousState) :
	AbstractState(stateData),
	m_previousState(std::move(previousState))
	{
	}
}
