// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_CHATCOMMANDSTORE_HPP
#define EREWHON_SHARED_CHATCOMMANDSTORE_HPP

#include <functional>
#include <map>
#include <optional>
#include <string>

namespace ewn
{
	template<typename Application, typename Client>
	class ChatCommandStore
	{
		public:
			inline ChatCommandStore(Application* app);
			~ChatCommandStore() = default;

			std::optional<bool> ExecuteCommand(Client* client, std::string_view cmd);

			template<typename... Args, typename... DefArgs> void RegisterCommand(std::string name, bool(*command)(Application* app, Client* client, Args...), DefArgs&&... defArgs);
			inline void UnregisterCommand(const std::string& name);

		private:
			using Command = std::function<bool(Application* app, Client* client, const std::string_view& cmd)>;

			void RegisterCommand(std::string name, Command cmd);

			std::map<std::string, Command, std::less<>> m_commands;
			Application* m_app;
	};
}

#include <Shared/ChatCommandStore.inl>

#endif // EREWHON_SHARED_CHATCOMMANDSTORE_HPP
