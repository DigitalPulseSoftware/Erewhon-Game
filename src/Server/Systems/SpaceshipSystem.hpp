// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_SPACESHIPSYSTEM_HPP
#define EREWHON_SERVER_SPACESHIPSYSTEM_HPP

#include <NDK/System.hpp>

namespace ewn
{
	class SpaceshipSystem : public Ndk::System<SpaceshipSystem>
	{
		public:
			SpaceshipSystem();

			static Ndk::SystemIndex systemIndex;

		private:
			void OnUpdate(float elapsedTime) override;
	};
}

#include <Server/Systems/SpaceshipSystem.inl>

#endif // EREWHON_SERVER_SPACESHIPSYSTEM_HPP
