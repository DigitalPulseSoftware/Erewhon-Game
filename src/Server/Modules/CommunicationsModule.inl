// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Modules/CommunicationsModule.hpp>

namespace ewn
{
	inline CommunicationsModule::CommunicationsModule(SpaceshipCore* core, const Ndk::EntityHandle& spaceship) :
	SpaceshipModule(ModuleType::Communications, core, spaceship, true),
	m_callbackCounter(0.f)
	{
	}
}

namespace Nz
{
	inline int LuaImplReplyVal(const LuaState& state, ewn::CommunicationsModule* ptr, TypeTag<ewn::CommunicationsModule*>)
	{
		state.PushInstance<ewn::CommunicationsModuleHandle>("Communications", ptr);
		return 1;
	}
}
