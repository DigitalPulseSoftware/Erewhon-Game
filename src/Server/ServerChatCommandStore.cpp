// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerChatCommandStore.hpp>
#include <Server/Arena.hpp>
#include <Server/Player.hpp>
#include <Server/ServerApplication.hpp>
#include <Server/Components/HealthComponent.hpp>

namespace ewn
{
	bool ChatCommandProcessArg(Player* player, std::string_view& cmdArgs, Player** arg, Nz::TypeTag<Player*>)
	{
		std::string playerName;
		if (!ChatCommandProcessArg(player, cmdArgs, &playerName, Nz::TypeTag<std::string>()))
			return false;

		if (Player* targetPlayer = player->GetArena()->FindPlayerByName(playerName))
		{
			*arg = targetPlayer;
			return true;
		}
		else
			return false;
	}

	void ServerChatCommandStore::BuildStore(ServerApplication* /*app*/)
	{
		RegisterCommand("crashserver", &ServerChatCommandStore::HandleCrashServer);
		RegisterCommand("kamikaze", &ServerChatCommandStore::HandleSuicide);
		RegisterCommand("kick", &ServerChatCommandStore::HandleKickPlayer);
		RegisterCommand("killbot", &ServerChatCommandStore::HandleKillBot);
		RegisterCommand("reloadmodules", &ServerChatCommandStore::HandleReloadModules);
		RegisterCommand("resetarena", &ServerChatCommandStore::HandleResetArena);
		RegisterCommand("suicide", &ServerChatCommandStore::HandleSuicide);
		RegisterCommand("spawnfleet", &ServerChatCommandStore::HandleSpawnFleet);
		RegisterCommand("stopserver", &ServerChatCommandStore::HandleStopServer);
		RegisterCommand("updatepermission", &ServerChatCommandStore::HandleUpdatePermission);
	}

	bool ServerChatCommandStore::HandleCrashServer(ServerApplication* /*app*/, Player* player)
	{
		if (player->GetPermissionLevel() < 40)
			return false;

		*static_cast<volatile int*>(nullptr) = 42;

		return true;
	}

	bool ServerChatCommandStore::HandleKickPlayer(ServerApplication* app, Player* player, Player* target)
	{
		if (player->GetPermissionLevel() < 30)
			return false;

		if (target->GetPermissionLevel() >= player->GetPermissionLevel())
		{
			player->PrintMessage("You're not allowed to do that");
			return false;
		}

		target->Disconnect();
		return true;
	}

	bool ServerChatCommandStore::HandleKillBot(ServerApplication* /*app*/, Player* player)
	{
		if (const Ndk::EntityHandle& botEntity = player->GetBotEntity())
			botEntity->Kill();

		return true;
	}

	bool ServerChatCommandStore::HandleReloadModules(ServerApplication* app, Player* player)
	{
		if (player->GetPermissionLevel() < 30)
			return false;

		ModuleStore& moduleStore = app->GetModuleStore();
		moduleStore.LoadFromDatabase(app, app->GetGlobalDatabase(), [ply = player->CreateHandle()](bool updateSucceeded)
		{
			if (updateSucceeded)
				ply->PrintMessage("Module reloaded");
			else
				ply->PrintMessage("Failed to reload modules");
		});

		return true;
	}

	bool ServerChatCommandStore::HandleResetArena(ServerApplication* /*app*/, Player* player)
	{
		if (player->GetPermissionLevel() < 20)
			return false;

		if (Arena* arena = player->GetArena())
			arena->Reset();

		return true;
	}

	bool ServerChatCommandStore::HandleSpawnFleet(ServerApplication* app, Player* player, std::string fleetName)
	{
		if (Arena* arena = player->GetArena())
			arena->SpawnFleet(player, fleetName);

		return true;
	}

	bool ServerChatCommandStore::HandleSuicide(ServerApplication* /*app*/, Player* player)
	{
		if (const Ndk::EntityHandle& playerSpaceship = player->GetControlledEntity())
		{
			HealthComponent& spaceshipHealth = playerSpaceship->GetComponent<HealthComponent>();
			spaceshipHealth.Damage(spaceshipHealth.GetHealth(), playerSpaceship);
		}

		return true;
	}

	bool ServerChatCommandStore::HandleStopServer(ServerApplication* app, Player* player)
	{
		if (player->GetPermissionLevel() < 40)
			return false;

		app->Quit();
		return true;
	}

	bool ServerChatCommandStore::HandleUpdatePermission(ServerApplication* app, Player* player, Player* target, Nz::UInt16 permissionLevel)
	{
		if (permissionLevel >= player->GetPermissionLevel())
		{
			player->PrintMessage("You're not allowed to do that");
			return false;
		}

		if (target->GetPermissionLevel() >= player->GetPermissionLevel())
		{
			player->PrintMessage("You're not allowed to do that");
			return false;
		}

		target->UpdatePermissionLevel(permissionLevel, [ply = player->CreateHandle()](bool success)
		{
			if (!ply)
				return;

			if (success)
				ply->PrintMessage("Permission level was successfully updated");
			else
				ply->PrintMessage("Failed to update permission level in database, changes are local");
		});

		return false;
	}
}
