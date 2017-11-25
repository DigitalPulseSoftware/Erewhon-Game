// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerApplication.hpp>
#include <Server/Player.hpp>
#include <iostream>

namespace ewn
{
	ServerApplication::ServerApplication() :
	m_playerPool(sizeof(Player)),
	m_arena(this),
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

		m_players[peerId] = m_playerPool.New<Player>(peerId, *reactor, m_commandStore);
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

		if (data.login.empty())
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

		static constexpr std::size_t MaxChatLine = 255;

		Nz::String message = player->GetName() + ": " + data.text;
		if (message.GetSize() > MaxChatLine)
		{
			message.Resize(MaxChatLine - 3, Nz::String::HandleUtf8);
			message += "...";
		}

		std::cout << message << std::endl;

		player->GetArena()->DispatchChatMessage(player, message);
	}

	void ServerApplication::HandlePlayerMovement(std::size_t peerId, const Packets::PlayerMovement& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		// TODO: Set speed limit accordingly to spaceship data
		Nz::Vector3f directionSpeed;
		directionSpeed.x = Nz::Clamp(data.direction.x, -50.f, 50.f);
		directionSpeed.y = Nz::Clamp(data.direction.y, -50.f, 50.f);
		directionSpeed.z = Nz::Clamp(data.direction.z, -50.f, 50.f);

		Nz::Vector3f rotationSpeed;
		rotationSpeed.x = Nz::Clamp(data.rotation.x, -200.f, 200.f);
		rotationSpeed.y = Nz::Clamp(data.rotation.y, -200.f, 200.f);
		rotationSpeed.z = Nz::Clamp(data.rotation.z, -200.f, 200.f);

		player->UpdateInput(directionSpeed, rotationSpeed);
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
}
