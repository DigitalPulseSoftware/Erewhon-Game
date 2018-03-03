// Copyright (C) 2017 Jérôme Leclercq
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
		RegisterCommand("resetarena", &ServerChatCommandStore::HandleResetArena);
		RegisterCommand("suicide", &ServerChatCommandStore::HandleSuicide);
		RegisterCommand("stopserver", &ServerChatCommandStore::HandleStopServer);
	}

	bool ServerChatCommandStore::HandleCrashServer(ServerApplication* /*app*/, Player* player)
	{
		// Dat security again
		if (player->GetName() != "Lynix")
			return false;

		*static_cast<volatile int*>(nullptr) = 42;

		return true;
	}

	bool ServerChatCommandStore::HandleKickPlayer(ServerApplication* app, Player* player, Player* target)
	{
		if (player->GetName() != "Lynix")
			return false;

		target->Disconnect();
		return true;
	}

	bool ServerChatCommandStore::HandleKillBot(ServerApplication* /*app*/, Player* player)
	{
		if (const Ndk::EntityHandle& botEntity = player->GetBotEntity())
			botEntity->Kill();

		return true;
	}

	bool ServerChatCommandStore::HandleResetArena(ServerApplication* /*app*/, Player* player)
	{
		// Dat security again
		if (player->GetName() != "Lynix")
			return false;

		if (Arena* arena = player->GetArena())
			arena->Reset();

		return true;
	}

	bool ServerChatCommandStore::HandleSuicide(ServerApplication* /*app*/, Player* player)
	{
		if (const Ndk::EntityHandle& playerSpaceship = player->GetControlledSpaceship())
		{
			HealthComponent& spaceshipHealth = playerSpaceship->GetComponent<HealthComponent>();
			spaceshipHealth.Damage(spaceshipHealth.GetHealth(), playerSpaceship);
		}

		return true;
	}

	bool ServerChatCommandStore::HandleStopServer(ServerApplication* app, Player* player)
	{
		// Dat security again
		if (player->GetName() != "Lynix")
			return false;

		app->Quit();
		return true;
	}
}
