// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_LIFETIMESYSTEM_HPP
#define EREWHON_SERVER_LIFETIMESYSTEM_HPP

#include <NDK/System.hpp>

namespace ewn
{
	class LifeTimeSystem : public Ndk::System<LifeTimeSystem>
	{
		public:
			LifeTimeSystem();
			~LifeTimeSystem() = default;

			static Ndk::SystemIndex systemIndex;

		private:
			void OnUpdate(float elapsedTime) override;
	};
}

#include <Server/Systems/LifeTimeSystem.inl>

#endif // EREWHON_SERVER_LIFETIMESYSTEM_HPP
