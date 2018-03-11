// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_CHATCOMMANDSTORE_HPP
#define EREWHON_SERVER_CHATCOMMANDSTORE_HPP

#include <Shared/ChatCommandStore.hpp>

namespace ewn
{
	class Player;
	class ServerApplication;

	class ServerChatCommandStore final : public ChatCommandStore<ServerApplication, Player>
	{
		public:
			inline ServerChatCommandStore(ServerApplication* app);
			~ServerChatCommandStore() = default;

		private:
			void BuildStore(ServerApplication* app);

			static bool HandleCrashServer(ServerApplication* app, Player* player);
			static bool HandleKickPlayer(ServerApplication* app, Player* player, Player* target);
			static bool HandleKillBot(ServerApplication* app, Player* player);
			static bool HandleReloadModules(ServerApplication* app, Player* player);
			static bool HandleResetArena(ServerApplication* app, Player* player);
			static bool HandleSuicide(ServerApplication* app, Player* player);
			static bool HandleStopServer(ServerApplication* app, Player* player);
			static bool HandleUpdatePermission(ServerApplication* app, Player* player, Player* target, Nz::UInt16 permissionLevel);
	};
}

#include <Server/ServerChatCommandStore.inl>

#endif // EREWHON_SERVER_CHATCOMMANDSTORE_HPP
