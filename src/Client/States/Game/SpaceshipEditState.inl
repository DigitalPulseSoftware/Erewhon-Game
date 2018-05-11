// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/SpaceshipEditState.hpp>

namespace ewn
{
	inline SpaceshipEditState::SpaceshipEditState(StateData & stateData, std::shared_ptr<Ndk::State> previousState) :
	AbstractState(stateData),
	m_previousState(std::move(previousState))
	{
	}

	inline SpaceshipEditState::SpaceshipEditState(StateData& stateData, std::shared_ptr<Ndk::State> previousState, std::string spaceshipName) :
	SpaceshipEditState(stateData, std::move(previousState))
	{
		m_tempSpaceshipName = std::move(spaceshipName);
	}

	inline bool SpaceshipEditState::IsInEditMode() const
	{
		return !m_spaceshipName.empty();
	}
}
