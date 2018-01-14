// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/SpaceshipCore.hpp>

namespace ewn
{
	inline SpaceshipCore::SpaceshipCore(const Ndk::EntityHandle& spaceship) :
	m_spaceship(spaceship)
	{
	}

	inline void SpaceshipCore::AddModule(std::unique_ptr<SpaceshipModule> modulePtr)
	{
		m_modules.emplace_back(std::move(modulePtr));
	}
}

namespace Nz
{
	inline int LuaImplReplyVal(const LuaState& state, ewn::SpaceshipCore* ptr, TypeTag<ewn::SpaceshipCore*>)
	{
		state.PushInstance<ewn::SpaceshipCoreHandle>("Core", ptr);
		return 1;
	}
}
