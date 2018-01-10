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
}

namespace Nz
{
	inline int LuaImplReplyVal(const LuaState& state, ewn::SpaceshipCore* ptr, TypeTag<ewn::SpaceshipCore*>)
	{
		state.PushInstance<ewn::SpaceshipCoreHandle>("Core", ptr);
		return 1;
	}
}
