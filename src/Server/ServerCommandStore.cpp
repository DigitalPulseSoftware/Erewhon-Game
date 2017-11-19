// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerCommandStore.hpp>
#include <Server/ServerApplication.hpp>

namespace ewn
{
	ServerCommandStore::ServerCommandStore(ServerApplication* app)
	{
		using namespace std::placeholders;

#define IncomingCommand(Type, Func) RegisterIncomingCommand<Packets::Type>(#Type, Func)
#define OutgoingCommand(Type, Flags, Channel) RegisterOutgoingCommand<Packets::Type>(#Type, Flags, Channel)

		// Incoming commands
		IncomingCommand(JoinArena,       std::bind(&ServerApplication::HandleJoinArena, app, _1, _2));
		IncomingCommand(Login,           std::bind(&ServerApplication::HandleLogin, app, _1, _2));
		IncomingCommand(PlayerMovement,  std::bind(&ServerApplication::HandlePlayerMovement, app, _1, _2));
		IncomingCommand(TimeSyncRequest, std::bind(&ServerApplication::HandleTimeSyncRequest, app, _1, _2));

		// Outgoing commands
		OutgoingCommand(ArenaState,       0,                           0);
		OutgoingCommand(ControlSpaceship, Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(CreateSpaceship,  Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(DeleteSpaceship,  Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(LoginFailure,     Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(LoginSuccess,     Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(TimeSyncResponse, 0, 0);

#undef IncomingCommand
#undef OutgoingCommand
	}
}
