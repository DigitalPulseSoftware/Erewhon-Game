// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Modules/NavigationModule.hpp>
#include <NDK/LuaAPI.hpp>
#include <Server/Components/NavigationComponent.hpp>
#include <Server/Modules/RadarModule.hpp>
#include <iostream>

namespace ewn
{
	void NavigationModule::Initialize(Ndk::Entity* spaceship)
	{
		spaceship->AddComponent<NavigationComponent>();
	}

	void NavigationModule::PushInstance(Nz::LuaState& lua)
	{
		lua.Push(this);
	}
	
	void NavigationModule::RegisterModule(Nz::LuaClass<SpaceshipModule>& parentBinding, Nz::LuaState& lua)
	{
		if (!s_binding)
		{
			s_binding.emplace("Navigation");
			s_binding->Inherit<SpaceshipModule>(parentBinding, [](NavigationModuleHandle* moduleRef) -> SpaceshipModule*
			{
				return moduleRef->GetObject();
			});

			s_binding->BindMethod("FollowTarget", [](Nz::LuaState& state, NavigationModule* navigation, std::size_t argCount)
			{
				int argIndex = 2;
				switch (argCount)
				{
					case 0:
					case 1:
					{
						Nz::Int64 signature = state.Check<Nz::Int64>(&argIndex);
						navigation->FollowTarget(signature);
						break;
					}

					case 2:
					default:
					{
						Nz::Int64 signature = state.Check<Nz::Int64>(&argIndex);
						float triggerDistance = state.Check<float>(&argIndex);
						navigation->FollowTarget(signature, triggerDistance);
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

			s_binding->BindMethod("OrientToPosition", &NavigationModule::OrientToPosition);
			s_binding->BindMethod("OrientToTarget", &NavigationModule::OrientToTarget);

			s_binding->BindMethod("Stop", &NavigationModule::Stop);
		}

		s_binding->Register(lua);
	}

	void NavigationModule::FollowTarget(Nz::Int64 targetSignature)
	{
		RadarModule* radarModule = GetCore()->GetModule<RadarModule>(ModuleType::Radar);
		if (!radarModule)
			return;

		const Ndk::EntityHandle& spaceship = GetSpaceship();
		NavigationComponent& spaceshipNavigation = spaceship->GetComponent<NavigationComponent>();
		if (const Ndk::EntityHandle& target = radarModule->FindEntityBySignature(targetSignature))
			spaceshipNavigation.SetTarget(target);
		else
			spaceshipNavigation.ClearTarget();
	}

	void NavigationModule::FollowTarget(Nz::Int64 targetSignature, float triggerDistance)
	{
		RadarModule* radarModule = GetCore()->GetModule<RadarModule>(ModuleType::Radar);
		if (!radarModule)
			return;

		const Ndk::EntityHandle& spaceship = GetSpaceship();
		NavigationComponent& spaceshipNavigation = spaceship->GetComponent<NavigationComponent>();
		if (const Ndk::EntityHandle& target = radarModule->FindEntityBySignature(targetSignature))
		{
			spaceshipNavigation.SetTarget(target, triggerDistance, [moduleHandle = CreateHandle()]()
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

	void NavigationModule::OrientToPosition(const Nz::Vector3f & targetPos)
	{
		NavigationComponent& spaceshipNavigation = GetSpaceship()->GetComponent<NavigationComponent>();
		spaceshipNavigation.SetTarget(targetPos, false);
	}

	void NavigationModule::OrientToTarget(Nz::Int64 targetSignature)
	{
		RadarModule* radarModule = GetCore()->GetModule<RadarModule>(ModuleType::Radar);
		if (!radarModule)
			return;

		const Ndk::EntityHandle& spaceship = GetSpaceship();
		NavigationComponent& spaceshipNavigation = spaceship->GetComponent<NavigationComponent>();
		if (const Ndk::EntityHandle& target = radarModule->FindEntityBySignature(targetSignature))
			spaceshipNavigation.SetTarget(target, false);
	}

	void NavigationModule::Stop()
	{
		NavigationComponent& spaceshipNavigation = GetSpaceship()->GetComponent<NavigationComponent>();
		spaceshipNavigation.ClearTarget();
	}

	std::optional<Nz::LuaClass<NavigationModuleHandle>> NavigationModule::s_binding;
}
