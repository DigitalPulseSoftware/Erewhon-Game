// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/SpaceshipCore.hpp>
#include <NDK/Components/PhysicsComponent3D.hpp>
#include <Server/SpaceshipModule.hpp>
#include <Server/Components/HealthComponent.hpp>
#include <iostream>

namespace ewn
{
	SpaceshipCore::~SpaceshipCore() = default;

	void SpaceshipCore::AddModule(std::shared_ptr<SpaceshipModule> newModule)
	{
		const auto& modulePtr = m_modules.emplace_back(std::move(newModule));

		if (modulePtr->IsRunnable())
			m_runnableModules.emplace_back(modulePtr);

		modulePtr->Initialize(m_spaceship);
	}

	LuaVec3 SpaceshipCore::GetAngularVelocity() const
	{
		auto& nodeComponent = m_spaceship->GetComponent<Ndk::PhysicsComponent3D>();
		return LuaVec3(nodeComponent.GetAngularVelocity());
	}

	float SpaceshipCore::GetIntegrity() const
	{
		auto& healthComponent = m_spaceship->GetComponent<HealthComponent>();
		return healthComponent.GetHealthPct();
	}

	LuaVec3 SpaceshipCore::GetLinearVelocity() const
	{
		auto& nodeComponent = m_spaceship->GetComponent<Ndk::PhysicsComponent3D>();
		return LuaVec3(nodeComponent.GetLinearVelocity());
	}

	LuaVec3 SpaceshipCore::GetPosition() const
	{
		auto& nodeComponent = m_spaceship->GetComponent<Ndk::PhysicsComponent3D>();
		return LuaVec3(nodeComponent.GetPosition());
	}

	LuaQuaternion SpaceshipCore::GetRotation() const
	{
		auto& nodeComponent = m_spaceship->GetComponent<Ndk::PhysicsComponent3D>();
		return LuaQuaternion(nodeComponent.GetRotation());
	}

	void SpaceshipCore::Register(Nz::LuaState& lua)
	{
		if (!s_binding)
		{
			s_binding.emplace("Core");

			s_binding->BindMethod("GetAngularVelocity", &SpaceshipCore::GetAngularVelocity);
			s_binding->BindMethod("GetIntegrity",       &SpaceshipCore::GetIntegrity);
			s_binding->BindMethod("GetLinearVelocity",  &SpaceshipCore::GetLinearVelocity);
			s_binding->BindMethod("GetPosition",        &SpaceshipCore::GetPosition);
			s_binding->BindMethod("GetRotation",        &SpaceshipCore::GetRotation);
		}

		s_binding->Register(lua);

		for (auto& modulePtr : m_modules)
			modulePtr->Register(lua);

		lua.PushField("Core", this);
	}

	void SpaceshipCore::Run()
	{
		for (const auto& modulePtr : m_runnableModules)
			modulePtr->Run();
	}

	std::optional<Nz::LuaClass<SpaceshipCoreHandle>> SpaceshipCore::s_binding;
}
