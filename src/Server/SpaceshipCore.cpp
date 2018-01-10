// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/SpaceshipCore.hpp>
#include <Server/Components/HealthComponent.hpp>

namespace ewn
{
	float SpaceshipCore::GetIntegrity() const
	{
		auto& healthComponent = m_spaceship->GetComponent<HealthComponent>();
		return healthComponent.GetHealthPct();
	}

	void SpaceshipCore::Register(Nz::LuaState& lua)
	{
		if (!s_binding)
		{
			s_binding.emplace("Core");

			s_binding->BindMethod("GetIntegrity", &SpaceshipCore::GetIntegrity);
		}

		s_binding->Register(lua);

		for (auto& modulePtr : m_modules)
			modulePtr->Register(lua);

		lua.PushField("Core", this);
	}

	std::optional<Nz::LuaClass<SpaceshipCore>> SpaceshipCore::s_binding;
}
