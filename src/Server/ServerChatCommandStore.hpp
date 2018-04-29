// Copyright (C) 2018 Jérôme Leclercq
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

			static bool HandleClearBots(ServerApplication* app, Player* player);
			static bool HandleCrashServer(ServerApplication* app, Player* player);
			static bool HandleDebugParticles(ServerApplication* app, Player* player, unsigned int particleSystemId);
			static bool HandleKickPlayer(ServerApplication* app, Player* player, Player* target);
			static bool HandleReloadModules(ServerApplication* app, Player* player);
			static bool HandleResetArena(ServerApplication* app, Player* player);
			static bool HandleSpawnFleet(ServerApplication* app, Player* player, std::string fleetName);
			static bool HandleSuicide(ServerApplication* app, Player* player);
			static bool HandleStopServer(ServerApplication* app, Player* player);
			static bool HandleUpdatePermission(ServerApplication* app, Player* player, Player* target, Nz::UInt16 permissionLevel);
	};
}

#include <Server/ServerChatCommandStore.inl>

#endif // EREWHON_SERVER_CHATCOMMANDSTORE_HPP
