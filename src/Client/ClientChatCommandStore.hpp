// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_CHATCOMMANDSTORE_HPP
#define EREWHON_CLIENT_CHATCOMMANDSTORE_HPP

#include <Shared/ChatCommandStore.hpp>

namespace ewn
{
	class ClientApplication;
	class ServerConnection;

	class ClientChatCommandStore final : public ChatCommandStore<ClientApplication, ServerConnection>
	{
		public:
			inline ClientChatCommandStore(ClientApplication* app);
			~ClientChatCommandStore() = default;

		private:
			void BuildStore(ClientApplication* app);

			static bool HandleCreateBot(ClientApplication* app, ServerConnection* server, std::string botName, const std::string& scriptName);
			static bool HandleDeleteBot(ClientApplication* app, ServerConnection* server, std::string botName);
			static bool HandleSpawnBot(ClientApplication* app, ServerConnection* server, std::string botName);
	};
}

#include <Client/ClientChatCommandStore.inl>

#endif // EREWHON_CLIENT_CHATCOMMANDSTORE_HPP
