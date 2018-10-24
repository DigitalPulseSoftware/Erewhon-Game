// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Scripting/ArenaInterface.hpp>
#include <NDK/LuaAPI.hpp>
#include <NDK/LuaAPI.hpp>
#include <Shared/Utils.hpp>
#include <Server/Arena.hpp>
#include <Server/Player.hpp>

namespace ewn
{
	void ArenaInterface::BindArena()
	{
		s_arenaBinding.Reset("Arena");
		
		s_arenaBinding.BindMethod("CreateEntity", &Arena::CreateEntity);
		s_arenaBinding.BindMethod("CreateSpaceship", &Arena::CreateSpaceship);
		s_arenaBinding.BindMethod("FindPlayerByName", &Arena::FindPlayerByName);
		s_arenaBinding.BindMethod("GetName", &Arena::GetName);
		s_arenaBinding.BindMethod("HandleChatMessage", &Arena::HandleChatMessage);
		s_arenaBinding.BindMethod("PrintChatMessage", &Arena::PrintChatMessage);
		s_arenaBinding.BindMethod("Reset", &Arena::Reset);
		s_arenaBinding.BindMethod("SpawnFleet", Overload<Player*, const std::string&>(&Arena::SpawnFleet));
		s_arenaBinding.BindMethod("SpawnSpaceship", Overload<Player*, std::string, std::size_t, const std::vector<std::size_t>&, const Nz::Vector3f&, const Nz::Quaternionf&>(&Arena::SpawnSpaceship));
	}
}
