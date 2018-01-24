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

#define IncomingCommand(Type) RegisterIncomingCommand<Packets::Type>(#Type, [app](std::size_t peerId, const Packets::Type& packet) \
{ \
	app->Handle##Type(peerId, packet); \
})
#define OutgoingCommand(Type, Flags, Channel) RegisterOutgoingCommand<Packets::Type>(#Type, Flags, Channel)

		// Incoming commands
		IncomingCommand(JoinArena);
		IncomingCommand(Login);
		IncomingCommand(PlayerChat);
		IncomingCommand(PlayerMovement);
		IncomingCommand(PlayerShoot);
		IncomingCommand(TimeSyncRequest);
		IncomingCommand(UploadScript);

		// Outgoing commands
		OutgoingCommand(ArenaState,       0,                           0);
		OutgoingCommand(BotMessage,       Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(ChatMessage,      Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(ControlEntity,    Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(CreateEntity,     Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(DeleteEntity,     Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(IntegrityUpdate,  Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(LoginFailure,     Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(LoginSuccess,     Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(TimeSyncResponse, 0,                           0);

#undef IncomingCommand
#undef OutgoingCommand
	}
}
