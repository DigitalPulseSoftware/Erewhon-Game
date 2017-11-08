// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
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
		IncomingCommand(Login, std::bind(&ServerApplication::HandleLogin, app, _1, _2));

		// Outgoing commands
		OutgoingCommand(LoginFailure, Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(LoginSuccess, Nz::ENetPacketFlag_Reliable, 0);

#undef IncomingCommand
#undef OutgoingCommand
	}
}
