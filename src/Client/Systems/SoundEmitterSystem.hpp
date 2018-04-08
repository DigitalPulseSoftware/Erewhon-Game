// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_SOUNDEMITTERSYSTEM_HPP
#define EREWHON_CLIENT_SOUNDEMITTERSYSTEM_HPP

#include <NDK/System.hpp>

namespace ewn
{
	class SoundEmitterSystem : public Ndk::System<SoundEmitterSystem>
	{
		public:
			SoundEmitterSystem();
			~SoundEmitterSystem() = default;

			static Ndk::SystemIndex systemIndex;

		private:
			void OnEntityAdded(Ndk::Entity* entity) override;
			void OnUpdate(float elapsedTime) override;
	};
}

#include <Server/Systems/LifeTimeSystem.inl>

#endif // EREWHON_CLIENT_SOUNDEMITTERSYSTEM_HPP
