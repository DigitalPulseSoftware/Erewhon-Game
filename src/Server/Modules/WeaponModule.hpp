// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_WEAPONMODULE_HPP
#define EREWHON_SERVER_WEAPONMODULE_HPP

#include <Nazara/Lua/LuaClass.hpp>
#include <Server/SpaceshipModule.hpp>
#include <optional>

namespace ewn
{
	class WeaponModule;

	using WeaponModuleHandle = Nz::ObjectHandle<WeaponModule>;

	class WeaponModule : public SpaceshipModule, public Nz::HandledObject<WeaponModule>
	{
		public:
			inline WeaponModule(SpaceshipCore* core, const Ndk::EntityHandle& spaceship);
			~WeaponModule() = default;

			void PushInstance(Nz::LuaState& lua) override;
			void RegisterModule(Nz::LuaClass<SpaceshipModule>& parentBinding, Nz::LuaState& lua) override;

			// Lua API
			void Shoot();

		protected:
			virtual void DoShoot() = 0;

		private:
			Nz::UInt64 m_lastShootTime;

			static std::optional<Nz::LuaClass<WeaponModuleHandle>> s_binding;
	};
}

#include <Server/Modules/WeaponModule.inl>

#endif // EREWHON_SERVER_WEAPONMODULE_HPP
