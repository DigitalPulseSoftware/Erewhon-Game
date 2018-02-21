// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_CHATCOMMANDSTORE_HPP
#define EREWHON_CLIENT_CHATCOMMANDSTORE_HPP

#include <functional>
#include <map>
#include <optional>
#include <string>

namespace ewn
{
	class ServerConnection;

	class ChatCommandStore
	{
		public:
			using Command = std::function<bool(ServerConnection* server)>;

			inline ChatCommandStore();
			~ChatCommandStore() = default;

			std::optional<bool> ExecuteCommand(const std::string_view& name, ServerConnection* server);

			inline void RegisterCommand(std::string name, Command command);
			inline void UnregisterCommand(const std::string& name);

		private:
			void BuildStore();

			static bool HandleUpload(ServerConnection* server);

			std::map<std::string, Command, std::less<>> m_commands;
	};
}

#include <Client/ChatCommandStore.inl>

#endif // EREWHON_CLIENT_CHATCOMMANDSTORE_HPP
