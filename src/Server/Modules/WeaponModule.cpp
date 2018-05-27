// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Modules/WeaponModule.hpp>
#include <Nazara/Core/Clock.hpp>

namespace ewn
{
	void WeaponModule::PushInstance(Nz::LuaState& lua)
	{
		lua.Push(this);
	}

	void WeaponModule::RegisterModule(Nz::LuaClass<SpaceshipModule>& parentBinding, Nz::LuaState& lua)
	{
		if (!s_binding)
		{
			s_binding.emplace("Weapon");
			s_binding->Inherit<SpaceshipModule>(parentBinding, [](WeaponModuleHandle* moduleRef) -> SpaceshipModule*
			{
				return moduleRef->GetObject();
			});

			s_binding->BindMethod("Shoot", &WeaponModule::Shoot);
		}

		s_binding->Register(lua);
	}

	void WeaponModule::Shoot()
	{
		Nz::UInt64 currentTime = Nz::GetElapsedMilliseconds();
		if (currentTime - m_lastShootTime < 500)
			return;

		m_lastShootTime = currentTime;

		DoShoot();
	}

	std::optional<Nz::LuaClass<WeaponModuleHandle>> WeaponModule::s_binding;
}
