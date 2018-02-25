// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ChatCommandStore.hpp>
#include <Server/Arena.hpp>
#include <Server/Player.hpp>
#include <Server/ServerApplication.hpp>
#include <Server/Components/HealthComponent.hpp>

#include <exception>

namespace ewn
{
	bool ChatCommandStore::ExecuteCommand(const std::string_view& name, Player* player)
	{
		auto it = m_commands.find(name);
		if (it != m_commands.end())
			return it->second(m_app, player);
		else
			return false;
	}

	void ChatCommandStore::BuildStore()
	{
		RegisterCommand("crashserver", &ChatCommandStore::HandleCrashServer);
		RegisterCommand("kamikaze", &ChatCommandStore::HandleSuicide);
		RegisterCommand("killbot", &ChatCommandStore::HandleKillBot);
		RegisterCommand("resetarena", &ChatCommandStore::HandleResetArena);
		RegisterCommand("suicide", &ChatCommandStore::HandleSuicide);
		RegisterCommand("stopserver", &ChatCommandStore::HandleStopServer);
	}

	bool ChatCommandStore::HandleCrashServer(ServerApplication* /*app*/, Player* player)
	{
		// Dat security again
		if (player->GetName() != "Lynix")
			return false;

		std::terminate();

		return true;
	}

	bool ChatCommandStore::HandleKillBot(ServerApplication * app, Player * player)
	{
		if (const Ndk::EntityHandle& botEntity = player->GetBotEntity())
			botEntity->Kill();

		return true;
	}

	bool ChatCommandStore::HandleResetArena(ServerApplication* /*app*/, Player* player)
	{
		// Dat security again
		if (player->GetName() != "Lynix")
			return false;

		if (Arena* arena = player->GetArena())
			arena->Reset();

		return true;
	}

	bool ChatCommandStore::HandleSuicide(ServerApplication* /*app*/, Player* player)
	{
		if (const Ndk::EntityHandle& playerSpaceship = player->GetControlledSpaceship())
		{
			HealthComponent& spaceshipHealth = playerSpaceship->GetComponent<HealthComponent>();
			spaceshipHealth.Damage(spaceshipHealth.GetHealth(), playerSpaceship);
		}

		return true;
	}

	bool ChatCommandStore::HandleStopServer(ServerApplication* app, Player* player)
	{
		// Dat security again
		if (player->GetName() != "Lynix")
			return false;

		app->Quit();
		return true;
	}
}
