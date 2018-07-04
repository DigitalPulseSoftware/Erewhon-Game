// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/FleetEditState.hpp>

namespace ewn
{
	inline FleetEditState::FleetEditState(StateData& stateData, std::shared_ptr<Ndk::State> previousState, std::string fleetName) :
	AbstractState(stateData),
	m_previousState(std::move(previousState))
	{
		if (!fleetName.empty())
		{
			m_fleetName = std::move(fleetName);
			m_isInEditMode = true;
		}
		else
			m_isInEditMode = false;
	}

	inline bool FleetEditState::IsAnySpaceshipInCollision() const
	{
		for (const auto& spaceship : m_spaceships)
			if (spaceship.hasCollisions)
				return true;

		return false;
	}

	inline bool FleetEditState::IsInEditMode() const
	{
		return m_isInEditMode;
	}

	inline float FleetEditState::Snap(float position)
	{
		return std::round(position);
	}

	inline Nz::Vector3f FleetEditState::SnapToGrid(Nz::Vector3f position)
	{
		constexpr float halfGridSize = (GridSize / GridScale) / 2.f;

		position.x = Nz::Clamp(Snap(position.x), -halfGridSize, halfGridSize);
		position.z = Nz::Clamp(Snap(position.z), -halfGridSize, halfGridSize);

		return position;
	}
}
