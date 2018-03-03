// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_SPACESHIPMODULE_HPP
#define EREWHON_SERVER_SPACESHIPMODULE_HPP

#include <Server/SpaceshipCore.hpp>
#include <Nazara/Lua/LuaInstance.hpp>
#include <NDK/Entity.hpp>
#include <optional>

namespace ewn
{
	class SpaceshipModule
	{
		public:
			inline SpaceshipModule(SpaceshipCore* core, const Ndk::EntityHandle& spaceship);
			virtual ~SpaceshipModule();

			inline void Disable();
			inline void Enable(bool enable = true);

			inline bool IsEnabled() const;

			virtual void Register(Nz::LuaState& lua) = 0;

		protected:
			inline const Ndk::EntityHandle& GetSpaceship();
			template<typename... Args> void PushCallback(Args&&... args);

		private:
			Ndk::EntityHandle m_spaceship;
			SpaceshipCoreHandle m_core;
			bool m_enabled;
	};
}

#include <Server/SpaceshipModule.inl>

#endif // EREWHON_SERVER_SPACESHIPMODULE_HPP
