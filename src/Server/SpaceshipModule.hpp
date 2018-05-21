// Copyright (C) 2018 Jérôme Leclercq
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
			inline SpaceshipModule(ModuleType moduleType, SpaceshipCore* core, const Ndk::EntityHandle& spaceship, bool runnable = false);
			virtual ~SpaceshipModule();

			inline void Disable();
			inline void Enable(bool enable = true);

			inline ModuleType GetType() const;

			virtual void Initialize(Ndk::Entity* spaceship);

			inline bool IsEnabled() const;
			inline bool IsRunnable() const;

			virtual void Register(Nz::LuaState& lua) = 0;
			virtual void Run(float elapsedTime);

		protected:
			inline SpaceshipCore* GetCore();
			inline const Ndk::EntityHandle& GetSpaceship();
			inline const Ndk::EntityHandle& GetSpaceship() const;
			template<typename... Args> void PushCallback(Args&&... args);

		private:
			Ndk::EntityHandle m_spaceship;
			ModuleType m_type;
			SpaceshipCoreHandle m_core;
			bool m_enabled;
			bool m_isRunnable;
	};
}

#include <Server/SpaceshipModule.inl>

#endif // EREWHON_SERVER_SPACESHIPMODULE_HPP
