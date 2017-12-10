// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/LifeTimeComponent.hpp>

namespace ewn
{
	inline LifeTimeComponent::LifeTimeComponent(float durationInSeconds) :
	m_remainingDuration(durationInSeconds)
	{
	}

	inline bool LifeTimeComponent::DecreaseDuration(float timeInSeconds)
	{
		m_remainingDuration -= timeInSeconds;
		return m_remainingDuration <= 0.f;
	}

	inline float LifeTimeComponent::GetRemainingDuration() const
	{
		return m_remainingDuration;
	}
}
