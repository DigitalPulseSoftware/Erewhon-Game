// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_CHATCOMMANDSTORE_HPP
#define EREWHON_SERVER_CHATCOMMANDSTORE_HPP

#include <functional>
#include <map>
#include <string>

namespace ewn
{
	class Player;
	class ServerApplication;

	class ChatCommandStore
	{
		public:
			using Command = std::function<bool(ServerApplication* app, Player* player)>;

			inline ChatCommandStore(ServerApplication* app);
			~ChatCommandStore() = default;

			bool ExecuteCommand(const std::string_view& name, Player* player);

			inline void RegisterCommand(std::string name, Command command);
			inline void UnregisterCommand(const std::string& name);

		private:
			void BuildStore();

			static bool HandleCrashServer(ServerApplication* app, Player* player);
			static bool HandleResetArena(ServerApplication* app, Player* player);
			static bool HandleStopServer(ServerApplication* app, Player* player);

			std::map<std::string, Command, std::less<>> m_commands;
			ServerApplication* m_app;
	};
}

#include <Server/ChatCommandStore.inl>

#endif // EREWHON_SERVER_CHATCOMMANDSTORE_HPP
