// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Scripting/ArenaInterface.hpp>

namespace ewn
{
	void ArenaInterface::Register(Nz::LuaState& lua)
	{
		s_arenaBinding.Register(lua);
		s_playerBinding.Register(lua);
	}

	bool ArenaInterface::Initialize()
	{
		BindArena();
		BindPlayer();

		return true;
	}

	void ArenaInterface::Uninitialize()
	{
		s_arenaBinding.Reset();
		s_playerBinding.Reset();
	}

	Nz::LuaClass<Arena*> ArenaInterface::s_arenaBinding;
	Nz::LuaClass<PlayerHandle> ArenaInterface::s_playerBinding;
}
