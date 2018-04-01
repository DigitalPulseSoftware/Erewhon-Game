// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Modules/NavigationModule.hpp>
#include <NDK/LuaAPI.hpp>
#include <Server/Components/NavigationComponent.hpp>
#include <Server/Components/RadarComponent.hpp>
#include <iostream>

namespace ewn
{
	void NavigationModule::FollowTarget(Ndk::EntityId targetId)
	{
		const Ndk::EntityHandle& spaceship = GetSpaceship();
		Ndk::World* world = spaceship->GetWorld();

		// FIXME: This should use the radar module instead of the radar component (to keep gameplay stuff inside gameplay classes)
		RadarComponent& spaceshipRadar = spaceship->GetComponent<RadarComponent>();
		if (!spaceshipRadar.IsEntityLocked(targetId))
			return;

		NavigationComponent& spaceshipNavigation = spaceship->GetComponent<NavigationComponent>();

		if (world->IsEntityIdValid(targetId))
			spaceshipNavigation.SetTarget(world->GetEntity(targetId));
		else
			spaceshipNavigation.ClearTarget();
	}

	void NavigationModule::FollowTarget(Ndk::EntityId targetId, float triggerDistance)
	{
		const Ndk::EntityHandle& spaceship = GetSpaceship();
		Ndk::World* world = spaceship->GetWorld();

		NavigationComponent& spaceshipNavigation = spaceship->GetComponent<NavigationComponent>();

		if (world->IsEntityIdValid(targetId))
		{
			spaceshipNavigation.SetTarget(world->GetEntity(targetId), triggerDistance, [moduleHandle = CreateHandle()]()
			{
				if (!moduleHandle)
					return;

				moduleHandle->PushCallback("OnNavigationDestinationReached");
			});
		}
		else
			spaceshipNavigation.ClearTarget();
	}

	void NavigationModule::MoveToPosition(const Nz::Vector3f& targetPos)
	{
		NavigationComponent& spaceshipNavigation = GetSpaceship()->GetComponent<NavigationComponent>();
		spaceshipNavigation.SetTarget(targetPos);
	}

	void NavigationModule::MoveToPosition(const Nz::Vector3f& targetPos, float triggerDistance)
	{
		NavigationComponent& spaceshipNavigation = GetSpaceship()->GetComponent<NavigationComponent>();
		spaceshipNavigation.SetTarget(targetPos, triggerDistance, [moduleHandle = CreateHandle()]()
		{
			if (!moduleHandle)
				return;

			moduleHandle->PushCallback("OnNavigationDestinationReached");
		});
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

			s_binding->BindMethod("FollowTarget", [](Nz::LuaState& state, NavigationModule* navigation, std::size_t argCount)
			{
				int argIndex = 2;
				switch (argCount)
				{
					case 0:
					case 1:
					{
						Ndk::EntityId entityId = state.Check<Ndk::EntityId>(&argIndex);
						navigation->FollowTarget(entityId);
						break;
					}

					case 2:
					default:
					{
						Ndk::EntityId entityId = state.Check<Ndk::EntityId>(&argIndex);
						float triggerDistance = state.Check<float>(&argIndex);
						navigation->FollowTarget(entityId, triggerDistance);
						break;
					}
				}

				return 0;
			});

			s_binding->BindMethod("MoveToPosition", [](Nz::LuaState& state, NavigationModule* navigation, std::size_t argCount)
			{
				int argIndex = 2;
				switch (argCount)
				{
					case 0:
					case 1:
					{
						Nz::Vector3f targetPos = state.Check<Nz::Vector3f>(&argIndex);
						navigation->MoveToPosition(targetPos);
						break;
					}

					case 2:
					default:
					{
						Nz::Vector3f targetPos = state.Check<Nz::Vector3f>(&argIndex);
						float triggerDistance = state.Check<float>(&argIndex);
						navigation->MoveToPosition(targetPos, triggerDistance);
						break;
					}
				}

				return 0;
			});

			s_binding->BindMethod("Stop", &NavigationModule::Stop);
		}

		s_binding->Register(lua);

		lua.PushField("Navigation", this);
	}

	void NavigationModule::Initialize(Ndk::Entity* spaceship)
	{
		spaceship->AddComponent<NavigationComponent>();
	}

	std::optional<Nz::LuaClass<NavigationModuleHandle>> NavigationModule::s_binding;
}
