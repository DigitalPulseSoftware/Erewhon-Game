// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ChatCommandStore.hpp>

namespace ewn
{
	inline ChatCommandStore::ChatCommandStore(ServerApplication* app) :
	m_app(app)
	{
		BuildStore();
	}

	inline void ChatCommandStore::RegisterCommand(std::string name, Command command)
	{
		m_commands.emplace(std::move(name), std::move(command));
	}

	inline void ChatCommandStore::UnregisterCommand(const std::string& name)
	{
		m_commands.erase(name);
	}
}
