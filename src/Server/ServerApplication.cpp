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

		m_players[peerId]->SetArena(&m_arena);
	}

	void ServerApplication::HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data)
	{
		std::cout << "Client #" << peerId << " disconnected with data " << data << std::endl;

		m_playerPool.Delete(m_players[peerId]);
		m_players[peerId] = nullptr;
	}

	void ServerApplication::HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet)
	{
		std::cout << "Client #" << peerId << " sent packet of size " << packet.GetDataSize() << std::endl;

		m_commandStore.UnserializePacket(peerId, std::move(packet));
	}

	void ServerApplication::HandleLogin(std::size_t peerId, const Packets::Login& data)
	{
		std::cout << "Player #" << peerId << " tried to login with\n";
		std::cout << " -login: " << data.login << '\n';
		std::cout << " -hash: " << data.passwordHash << std::endl;

		if (data.login == "Lynix" && data.passwordHash == "7a6f41c05125d270d4ad1893d81b61574445919c03ee4c7299d60328fccbd5be")
		{
			m_players[peerId]->SendPacket(Packets::LoginSuccess());
		}
		else
		{
			Packets::LoginFailure loginFailure;
			loginFailure.reason = 42;

			m_players[peerId]->SendPacket(loginFailure);
		}
	}

	void ServerApplication::HandlePlayerMovement(std::size_t peerId, const Packets::PlayerMovement& data)
	{
		//TODO: Check

		m_players[peerId]->UpdateInput(data.direction, data.rotation);
	}
}
