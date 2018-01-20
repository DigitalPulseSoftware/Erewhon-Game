// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerApplication.hpp>
#include <Server/Components/ScriptComponent.hpp>
#include <Server/Player.hpp>
#include <iostream>

namespace ewn
{
	ServerApplication::ServerApplication() :
	m_playerPool(sizeof(Player)),
	m_arena(this),
	m_chatCommandStore(this),
	m_commandStore(this)
	{
	}

	ServerApplication::~ServerApplication()
	{
		for (Player* player : m_players)
		{
			if (player)
			{
				player->Disconnect();
				m_playerPool.Delete(player);
			}
		}
	}

	bool ServerApplication::Run()
	{
		m_arena.Update(GetUpdateTime());

		return BaseApplication::Run();
	}

	void ServerApplication::HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data)
	{
		const std::unique_ptr<NetworkReactor>& reactor = GetReactor(peerId / GetPeerPerReactor());

		if (peerId >= m_players.size())
			m_players.resize(peerId + 1);

		m_players[peerId] = m_playerPool.New<Player>(this, peerId, *reactor, m_commandStore);
		std::cout << "Client #" << peerId << " connected with data " << data << std::endl;
	}

	void ServerApplication::HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data)
	{
		std::cout << "Client #" << peerId << " disconnected with data " << data << std::endl;

		m_playerPool.Delete(m_players[peerId]);
		m_players[peerId] = nullptr;
	}

	void ServerApplication::HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet)
	{
		//std::cout << "Client #" << peerId << " sent packet of size " << packet.GetDataSize() << std::endl;

		if (!m_commandStore.UnserializePacket(peerId, std::move(packet)))
			m_players[peerId]->Disconnect();
	}

	void ServerApplication::HandleLogin(std::size_t peerId, const Packets::Login& data)
	{
		std::cout << "Player #" << peerId << " tried to login with\n";
		std::cout << " -login: " << data.login << '\n';
		std::cout << " -hash: " << data.passwordHash << '\n';

		Player* player = m_players[peerId];
		if (player->IsAuthenticated())
			return;

		if (data.login.empty() || data.login.size() > 20)
			return;

		if (data.login != "Lynix" || data.passwordHash == "98b533129f885fefbf10b3b4678bca87989e55244e822df6b94fe29d96e89540")
		{
			player->Authenticate(data.login);

			player->SendPacket(Packets::LoginSuccess());

			std::cout << "Player #" << peerId << " authenticated as " << data.login << std::endl;
		}
		else
		{
			Packets::LoginFailure loginFailure;
			loginFailure.reason = 42;

			player->SendPacket(loginFailure);

			std::cout << "Player #" << peerId << " authenticated failed" << std::endl;
		}
	}

	void ServerApplication::HandleJoinArena(std::size_t peerId, const Packets::JoinArena& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		Arena* arena = &m_arena; //< One arena atm
		if (player->GetArena() != arena)
			player->MoveToArena(arena);
	}

	void ServerApplication::HandlePlayerChat(std::size_t peerId, const Packets::PlayerChat& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		if (data.text.empty())
			return;

		if (data.text[0] == '/')
		{
			std::string_view command = data.text;
			command.remove_prefix(1);

			if (m_chatCommandStore.ExecuteCommand(command, player))
				return; // Don't show command if it succeeded
		}

		if (Arena* arena = player->GetArena())
		{
			static constexpr std::size_t MaxChatLine = 255;

			Nz::String message = player->GetName() + ": " + data.text;
			if (message.GetSize() > MaxChatLine)
			{
				message.Resize(MaxChatLine - 3, Nz::String::HandleUtf8);
				message += "...";
			}

			std::cout << message << std::endl;

			arena->DispatchChatMessage(message);
		}
	}

	void ServerApplication::HandlePlayerMovement(std::size_t peerId, const Packets::PlayerMovement& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		player->UpdateInput(data.inputTime, data.direction, data.rotation);
	}

	void ServerApplication::HandlePlayerShoot(std::size_t peerId, const Packets::PlayerShoot& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		player->Shoot();
	}

	void ServerApplication::HandleTimeSyncRequest(std::size_t peerId, const Packets::TimeSyncRequest& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		Packets::TimeSyncResponse response;
		response.requestId = data.requestId;
		response.serverTime = GetAppTime();

		player->SendPacket(response);
	}

	void ServerApplication::HandleUploadScript(std::size_t peerId, const Packets::UploadScript& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		const Ndk::EntityHandle& playerBot = player->GetBotEntity();
		ScriptComponent& botScript = playerBot->AddComponent<ScriptComponent>();

		Nz::String lastError;
		if (botScript.Execute(data.code, &lastError))
		{
			Packets::ChatMessage chatPacket;
			chatPacket.message = "Server: Script uploaded with success";

			player->SendPacket(chatPacket);
		}
		else
		{
			Packets::ChatMessage chatPacket;
			chatPacket.message = "Server: Failed to execute script: " + lastError.ToStdString();

			player->SendPacket(chatPacket);
		}
	}
}
