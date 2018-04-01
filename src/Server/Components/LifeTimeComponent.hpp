// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_LIFETIMECOMPONENT_HPP
#define EREWHON_SERVER_LIFETIMECOMPONENT_HPP

#include <NDK/Component.hpp>

namespace ewn
{
	class LifeTimeComponent : public Ndk::Component<LifeTimeComponent>
	{
		public:
			inline LifeTimeComponent(float durationInSeconds);

			inline bool DecreaseDuration(float timeInSeconds);

			inline float GetRemainingDuration() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			float m_remainingDuration;
	};
}

#include <Server/Components/LifeTimeComponent.inl>

#endif // EREWHON_SERVER_LIFETIMECOMPONENT_HPP
