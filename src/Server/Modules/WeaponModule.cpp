// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Modules/WeaponModule.hpp>
#include <Nazara/Core/Clock.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <Server/Components/ArenaComponent.hpp>

namespace ewn
{
	void WeaponModule::Shoot()
	{
		Nz::UInt64 currentTime = Nz::GetElapsedMilliseconds();
		if (currentTime - m_lastShootTime < 500)
			return;

		m_lastShootTime = currentTime;

		const Ndk::EntityHandle& spaceship = GetSpaceship();
		auto& spaceshipNode = spaceship->GetComponent<Ndk::NodeComponent>();
		Arena& spaceshipArena = spaceship->GetComponent<ewn::ArenaComponent>();

		spaceshipArena.CreateProjectile(nullptr, spaceship, spaceshipNode.GetPosition() + spaceshipNode.GetForward() * 12.f, spaceshipNode.GetRotation());
	}

	void WeaponModule::Register(Nz::LuaState& lua)
	{
		if (!s_binding)
		{
			s_binding.emplace("Weapon");

			s_binding->BindMethod("Shoot", &WeaponModule::Shoot);
		}

		s_binding->Register(lua);

		lua.PushField("Weapon", this);
	}

	std::optional<Nz::LuaClass<WeaponModuleHandle>> WeaponModule::s_binding;
}
