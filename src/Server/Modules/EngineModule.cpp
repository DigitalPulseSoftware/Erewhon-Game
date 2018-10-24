// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Modules/EngineModule.hpp>
#include <NDK/LuaAPI.hpp>
#include <Server/Components/NavigationComponent.hpp>
#include <Server/ServerApplication.hpp>

namespace ewn
{
	void EngineModule::Impulse(Nz::Vector3f impulse, float duration)
	{
		impulse.x = Nz::Clamp(impulse.x, -1.f, 1.f);
		impulse.y = Nz::Clamp(impulse.y, -1.f, 1.f);
		impulse.z = Nz::Clamp(impulse.z, -1.f, 1.f);

		NavigationComponent& spaceshipNavigation = GetSpaceship()->GetComponent<NavigationComponent>();
		spaceshipNavigation.AddImpulse(impulse, GetCore()->GetApp()->GetAppTime() + Nz::UInt64(duration * 1'000));
	}

	void EngineModule::PushInstance(Nz::LuaState& lua)
	{
		lua.Push(this);
	}

	void EngineModule::RegisterModule(Nz::LuaClass<SpaceshipModule>& parentBinding, Nz::LuaState& lua)
	{
		if (!s_binding)
		{
			s_binding.emplace("Engine");
			s_binding->Inherit<SpaceshipModule>(parentBinding, [](EngineModuleHandle* moduleRef) -> SpaceshipModule*
			{
				return moduleRef->GetObject();
			});

			s_binding->BindMethod("Impulse", &EngineModule::Impulse);
		}

		s_binding->Register(lua);
	}

	std::optional<Nz::LuaClass<EngineModuleHandle>> EngineModule::s_binding;
}
