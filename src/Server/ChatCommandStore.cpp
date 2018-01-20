// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ChatCommandStore.hpp>
#include <Server/Arena.hpp>
#include <Server/Player.hpp>
#include <Server/ServerApplication.hpp>

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
		RegisterCommand("reload", &ChatCommandStore::HandleReload);
	}

	bool ChatCommandStore::HandleReload(ServerApplication* app, Player* player)
	{
		// Dat security
		if (player->GetName() != "Lynix")
			return false;

		if (Arena* arena = player->GetArena())
			arena->ReloadScripts();

		return true;
	}
}
