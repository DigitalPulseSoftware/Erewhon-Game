// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_NAVIGATIONSYSTEM_HPP
#define EREWHON_SERVER_NAVIGATIONSYSTEM_HPP

#include <NDK/System.hpp>

namespace ewn
{
	class ServerApplication;

	class NavigationSystem : public Ndk::System<NavigationSystem>
	{
		public:
			NavigationSystem();

			static Ndk::SystemIndex systemIndex;

		private:
			void OnUpdate(float elapsedTime) override;

			Nz::UInt64 m_appTime;
			float m_timeAccumulator;
	};
}

#include <Server/Systems/NavigationSystem.inl>

#endif // EREWHON_SERVER_NAVIGATIONSYSTEM_HPP
