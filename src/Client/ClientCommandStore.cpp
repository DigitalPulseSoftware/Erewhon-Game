// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientCommandStore.hpp>
#include <Client/ClientApplication.hpp>

namespace ewn
{
	ClientCommandStore::ClientCommandStore(ClientApplication* app)
	{
		using namespace std::placeholders;

#define IncomingCommand(Type, Func) RegisterIncomingCommand<Packets::Type>(#Type, Func)
#define OutgoingCommand(Type, Flags, Channel) RegisterOutgoingCommand<Packets::Type>(#Type, Flags, Channel)

		// Incoming commands
		IncomingCommand(ArenaState,        std::bind(&ClientApplication::HandleArenaState, app, _1, _2));
		IncomingCommand(ControlSpaceship,  std::bind(&ClientApplication::HandleControlSpaceship, app, _1, _2));
		IncomingCommand(CreateSpaceship,   std::bind(&ClientApplication::HandleCreateSpaceship, app, _1, _2));
		IncomingCommand(DeleteSpaceship,   std::bind(&ClientApplication::HandleDeleteSpaceship, app, _1, _2));
		IncomingCommand(LoginFailure,      std::bind(&ClientApplication::HandleLoginFailure, app, _1, _2));
		IncomingCommand(LoginSuccess,      std::bind(&ClientApplication::HandleLoginSuccess, app, _1, _2));

		// Outgoing commands
		OutgoingCommand(JoinArena,      Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(Login,          Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(PlayerMovement, 0,                           0);

#undef IncomingCommand
#undef OutgoingCommand
	}
}
