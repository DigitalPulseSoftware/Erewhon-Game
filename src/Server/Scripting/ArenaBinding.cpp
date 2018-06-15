// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Scripting/ArenaInterface.hpp>
#include <NDK/LuaAPI.hpp>
#include <Server/Arena.hpp>
#include <Server/Player.hpp>

namespace ewn
{
	void ArenaInterface::BindArena()
	{
		s_arenaBinding.Reset("Arena");
		
		s_arenaBinding.BindMethod("CreateEntity", &Arena::CreateEntity);
		s_arenaBinding.BindMethod("CreateSpaceship", &Arena::CreateSpaceship);
		s_arenaBinding.BindMethod("DispatchChatMessage", &Arena::DispatchChatMessage);
		s_arenaBinding.BindMethod("FindPlayerByName", &Arena::FindPlayerByName);
		s_arenaBinding.BindMethod("GetName", &Arena::GetName);
	}
}
