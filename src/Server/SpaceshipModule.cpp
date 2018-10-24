// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/SpaceshipModule.hpp>
#include <Server/SpaceshipCore.hpp>

namespace ewn
{
	SpaceshipModule::~SpaceshipModule() = default;

	void SpaceshipModule::Initialize(Ndk::Entity* spaceship)
	{
	}

	void SpaceshipModule::Register(Nz::LuaState& lua)
	{
		RegisterModule(s_binding.value(), lua);
	}

	void SpaceshipModule::RegisterParent(Nz::LuaState& lua)
	{
		if (!s_binding)
		{
			s_binding.emplace("Module");

			s_binding->BindMethod("GetType", &SpaceshipModule::GetType);
		}

		s_binding->Register(lua);
	}

	void SpaceshipModule::Run(float elapsedTime) 
	{
	}

	std::optional<Nz::LuaClass<SpaceshipModule>> SpaceshipModule::s_binding;
}
