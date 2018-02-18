// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Modules/NavigationModule.hpp>

namespace ewn
{
	inline NavigationModule::NavigationModule(const Ndk::EntityHandle & spaceship) :
	SpaceshipModule(spaceship)
	{
	}
}

namespace Nz
{
	inline int LuaImplReplyVal(const LuaState& state, ewn::NavigationModule* ptr, TypeTag<ewn::NavigationModule*>)
	{
		state.PushInstance<ewn::NavigationModuleHandle>("Navigation", ptr);
		return 1;
	}
}
