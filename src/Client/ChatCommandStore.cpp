// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ChatCommandStore.hpp>
#include <Nazara/Core/File.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Client/ClientApplication.hpp>
#include <iostream>

namespace ewn
{
	bool ChatCommandStore::ExecuteCommand(const std::string_view& name, ServerConnection* server)
	{
		auto it = m_commands.find(name);
		if (it != m_commands.end())
			return it->second(server);
		else
			return false;
	}

	void ChatCommandStore::BuildStore()
	{
		RegisterCommand("upload", &ChatCommandStore::HandleUpload);
	}

	bool ChatCommandStore::HandleUpload(ServerConnection* server)
	{
		static constexpr const char* scriptName = "botscript.lua";

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
