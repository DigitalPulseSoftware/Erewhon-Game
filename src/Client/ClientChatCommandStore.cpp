// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientChatCommandStore.hpp>
#include <Nazara/Core/File.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Client/ClientApplication.hpp>
#include <iostream>

namespace ewn
{
	void ClientChatCommandStore::BuildStore(ClientApplication* app)
	{
		RegisterCommand("createbot", &ClientChatCommandStore::HandleCreateBot, app->GetConfig().GetStringOption("ServerScript.Filename"));
		RegisterCommand("deletebot", &ClientChatCommandStore::HandleDeleteBot);
		RegisterCommand("spawnbot", &ClientChatCommandStore::HandleSpawnBot);
	}

	bool ClientChatCommandStore::HandleCreateBot(ClientApplication* app, ServerConnection* server, std::string botName, const std::string& scriptName)
	{
		Nz::File file(scriptName, Nz::OpenMode_ReadOnly | Nz::OpenMode_Text);
		if (!file.IsOpen())
		{
			std::cerr << "Failed to open " << scriptName << std::endl;
			return false;
		}

		Nz::String content;
		content.Reserve(file.GetSize());

		while (!file.EndOfFile())
		{
			content += file.ReadLine();
			content += '\n';
		}

		Packets::CreateSpaceship createSpaceshipPacket;
		createSpaceshipPacket.code = content.ToStdString();
		createSpaceshipPacket.spaceshipName = std::move(botName);

		server->SendPacket(createSpaceshipPacket);
		std::cout << "Code was successfully uploaded to the server" << std::endl;

		return true;
	}

	bool ClientChatCommandStore::HandleDeleteBot(ClientApplication* app, ServerConnection* server, std::string botName)
	{
		Packets::DeleteSpaceship packet;
		packet.spaceshipName = std::move(botName);

		server->SendPacket(packet);

		return true;
	}

	bool ClientChatCommandStore::HandleSpawnBot(ClientApplication* app, ServerConnection* server, std::string botName)
	{
		Packets::SpawnSpaceship packet;
		packet.spaceshipName = std::move(botName);

		server->SendPacket(packet);

		return true;
	}
}
