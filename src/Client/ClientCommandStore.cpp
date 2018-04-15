// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientCommandStore.hpp>
#include <Client/ServerConnection.hpp>

namespace ewn
{
	ClientCommandStore::ClientCommandStore(ServerConnection* server)
	{
		using namespace std::placeholders;

#define IncomingCommand(Type) RegisterIncomingCommand<Packets::Type>(#Type, [server](std::size_t peerId, const Packets::Type& data) \
{ \
	server->On##Type(server, data); \
})
#define OutgoingCommand(Type, Flags, Channel) RegisterOutgoingCommand<Packets::Type>(#Type, Flags, Channel)

		// Incoming commands
		IncomingCommand(ArenaPrefabs);
		IncomingCommand(ArenaSounds);
		IncomingCommand(ArenaState);
		IncomingCommand(BotMessage);
		IncomingCommand(ChatMessage);
		IncomingCommand(ControlEntity);
		IncomingCommand(CreateEntity);
		IncomingCommand(DeleteEntity);
		IncomingCommand(IntegrityUpdate);
		IncomingCommand(LoginFailure);
		IncomingCommand(LoginSuccess);
		IncomingCommand(NetworkStrings);
		IncomingCommand(PlaySound);
		IncomingCommand(RegisterFailure);
		IncomingCommand(RegisterSuccess);
		IncomingCommand(SpaceshipInfo);
		IncomingCommand(SpaceshipList);
		IncomingCommand(TimeSyncResponse);
		IncomingCommand(UpdateSpaceshipFailure);
		IncomingCommand(UpdateSpaceshipSuccess);

		// Outgoing commands
		OutgoingCommand(CreateSpaceship,    Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(DeleteSpaceship,    Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(JoinArena,          Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(Login,              Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(PlayerChat,         Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(PlayerMovement,     0,                           0);
		OutgoingCommand(PlayerShoot,        Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(QuerySpaceshipInfo, Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(QuerySpaceshipList, Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(Register,           Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(SpawnSpaceship,     Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(TimeSyncRequest,    0,                           0);
		OutgoingCommand(UpdateSpaceship,    Nz::ENetPacketFlag_Reliable, 0);

#undef IncomingCommand
#undef OutgoingCommand
	}
}
