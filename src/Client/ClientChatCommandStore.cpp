// Copyright (C) 2017 Jérôme Leclercq
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
		RegisterCommand("upload", &ClientChatCommandStore::HandleUpload, app->GetConfig().GetStringOption("ServerScript.Filename"));
	}

	bool ClientChatCommandStore::HandleUpload(ClientApplication* app, ServerConnection* server, const std::string& scriptName)
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

		Packets::UploadScript uploadPacket;
		uploadPacket.code = content.ToStdString();

		server->SendPacket(uploadPacket);
		std::cout << "Code was successfully uploaded to the server" << std::endl;

		return true;
	}
}
