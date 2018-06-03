// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Scripting/ArenaInterface.hpp>
#include <NDK/LuaAPI.hpp>
#include <Server/Arena.hpp>
#include <Server/Player.hpp>

namespace ewn
{
	void ArenaInterface::BindPlayer()
	{
		s_playerBinding.Reset("Player");
		
		s_playerBinding.BindMethod("ClearBots", &Player::ClearBots);
		s_playerBinding.BindMethod("ClearControlledEntity", &Player::ClearControlledEntity);
		s_playerBinding.BindMethod("Disconnect", &Player::Disconnect, Nz::UInt32(0));
		s_playerBinding.BindMethod("GetArena", &Player::GetArena);
		s_playerBinding.BindMethod("GetControlledEntity", &Player::GetControlledEntity);
		s_playerBinding.BindMethod("GetDatabaseId", &Player::GetDatabaseId);
		s_playerBinding.BindMethod("GetLastInputProcessedTime", &Player::GetLastInputProcessedTime);
		s_playerBinding.BindMethod("GetLogin", &Player::GetLogin);
		s_playerBinding.BindMethod("GetName", &Player::GetName);
		s_playerBinding.BindMethod("GetPeerId", &Player::GetPeerId);
		s_playerBinding.BindMethod("GetPermissionLevel", &Player::GetPermissionLevel);
		s_playerBinding.BindMethod("GetSessionId", &Player::GetSessionId);
		s_playerBinding.BindMethod("InstantiateBot", &Player::InstantiateBot);
		s_playerBinding.BindMethod("MoveToArena", &Player::MoveToArena);
		s_playerBinding.BindMethod("PrintMessage", &Player::PrintMessage);
		s_playerBinding.BindMethod("Shoot", &Player::Shoot);
		s_playerBinding.BindMethod("UpdateControlledEntity", &Player::UpdateControlledEntity);
	}
}
