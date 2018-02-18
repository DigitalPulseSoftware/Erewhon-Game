// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Modules/NavigationModule.hpp>
#include <NDK/LuaAPI.hpp>
#include <Server/Components/NavigationComponent.hpp>

namespace ewn
{
	void NavigationModule::FollowTarget(Ndk::EntityId targetId)
	{
		const Ndk::EntityHandle& spaceship = GetSpaceship();
		Ndk::World* world = spaceship->GetWorld();

		NavigationComponent& spaceshipNavigation = spaceship->GetComponent<NavigationComponent>();

		if (world->IsEntityIdValid(targetId))
			spaceshipNavigation.SetTarget(world->GetEntity(targetId));
		else
			spaceshipNavigation.ClearTarget();
	}

	void NavigationModule::MoveToPosition(const Nz::Vector3f& targetPos)
	{
		NavigationComponent& spaceshipNavigation = GetSpaceship()->GetComponent<NavigationComponent>();
		spaceshipNavigation.SetTarget(targetPos);
	}

	void NavigationModule::Stop()
	{
		NavigationComponent& spaceshipNavigation = GetSpaceship()->GetComponent<NavigationComponent>();
		spaceshipNavigation.ClearTarget();
	}

	void NavigationModule::Register(Nz::LuaState& lua)
	{
		if (!s_binding)
		{
			s_binding.emplace("Navigation");

			s_binding->BindMethod("FollowTarget", &NavigationModule::FollowTarget);
			s_binding->BindMethod("MoveToPosition", &NavigationModule::MoveToPosition);
			s_binding->BindMethod("Stop", &NavigationModule::Stop);
		}

		s_binding->Register(lua);

		lua.PushField("Navigation", this);
	}

	std::optional<Nz::LuaClass<NavigationModuleHandle>> NavigationModule::s_binding;
}
