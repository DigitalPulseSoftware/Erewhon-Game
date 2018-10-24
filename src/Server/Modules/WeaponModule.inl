// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Modules/WeaponModule.hpp>

namespace ewn
{
	inline WeaponModule::WeaponModule(SpaceshipCore* core, const Ndk::EntityHandle & spaceship) :
	SpaceshipModule(ModuleType::Weapon, core, spaceship),
	m_lastShootTime(0)
	{
	}
}

namespace Nz
{
	inline int LuaImplReplyVal(const LuaState& state, ewn::WeaponModule* ptr, TypeTag<ewn::WeaponModule*>)
	{
		state.PushInstance<ewn::WeaponModuleHandle>("Weapon", ptr);
		return 1;
	}
}
