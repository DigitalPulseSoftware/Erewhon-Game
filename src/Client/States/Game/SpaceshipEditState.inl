// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/SpaceshipEditState.hpp>

namespace ewn
{
	inline SpaceshipEditState::SpaceshipEditState(CreateMode /*mode*/, StateData& stateData, std::shared_ptr<Ndk::State> previousState, std::string hullModelPath, Nz::UInt32 hullId) :
	AbstractState(stateData),
	m_previousState(std::move(previousState)),
	m_spaceshipHullId(hullId),
	m_spaceshipModelPath(std::move(hullModelPath)),
	m_isInEditMode(false)
	{
	}

	inline SpaceshipEditState::SpaceshipEditState(EditMode /*mode*/, StateData& stateData, std::shared_ptr<Ndk::State> previousState, std::string spaceshipName) :
	AbstractState(stateData),
	m_previousState(std::move(previousState)),
	m_spaceshipHullId(0xFFFFFFFF)
	{
		m_spaceshipName = std::move(spaceshipName);
		m_isInEditMode = true;
	}

	inline bool SpaceshipEditState::IsInEditMode() const
	{
		return m_isInEditMode;
	}
}
