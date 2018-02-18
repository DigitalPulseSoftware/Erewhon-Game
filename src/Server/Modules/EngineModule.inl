// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Modules/EngineModule.hpp>

namespace ewn
{
	inline EngineModule::EngineModule(const Ndk::EntityHandle & spaceship) :
	SpaceshipModule(spaceship)
	{
	}
}

namespace Nz
{
	inline int LuaImplReplyVal(const LuaState& state, ewn::EngineModule* ptr, TypeTag<ewn::EngineModule*>)
	{
		state.PushInstance<ewn::EngineModuleHandle>("Engine", ptr);
		return 1;
	}
}
