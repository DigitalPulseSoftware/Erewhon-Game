// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Arena.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <Server/Player.hpp>
#include <Server/Components/PlayerControlledComponent.hpp>
#include <Server/ServerApplication.hpp>
#include <Server/Systems/SpaceshipSystem.hpp>
#include <cassert>

namespace ewn
{
	static constexpr bool sendServerGhosts = true;

	Arena::Arena(ServerApplication* app) :
	m_app(app),
	m_stateId(0),
	m_ghostBroadcastAccumulator(0.f),
	m_stateBroadcastAccumulator(0.f)
	{
		m_world.AddSystem<SpaceshipSystem>();

		if constexpr (sendServerGhosts)
		{
			m_debugSocket.Create(Nz::NetProtocol_IPv4);
			m_debugSocket.EnableBroadcasting(true);
		}
	}

	const Ndk::EntityHandle& Arena::CreatePlayerSpaceship(Player* player)
	{
		assert(m_players.find(player) != m_players.end());

		const Ndk::EntityHandle& spaceship = m_world.CreateEntity();
		auto& nodeComponent = spaceship->AddComponent<Ndk::NodeComponent>();
		spaceship->AddComponent<PlayerControlledComponent>();

		m_spaceships.Insert(spaceship);
		spaceship->OnEntityDestruction.Connect(this, &Arena::OnSpaceshipDestroy);

		m_players[player] = spaceship;

		// Create spaceship
		Packets::CreateSpaceship createSpaceship;
		createSpaceship.id = spaceship->GetId();
		createSpaceship.position = nodeComponent.GetPosition();
		createSpaceship.rotation = nodeComponent.GetRotation();
		createSpaceship.name = player->GetName();
		for (auto& pair : m_players)
			pair.first->SendPacket(createSpaceship);

		// Control packet
		Packets::ControlSpaceship controlPacket;
		controlPacket.id = spaceship->GetId();

		player->SendPacket(controlPacket);

		return spaceship;
	}

	void Arena::DispatchChatMessage(Player* player, const Nz::String& message)
	{
		Packets::ChatMessage chatPacket;
		chatPacket.message = message;

		for (auto& pair : m_players)
			pair.first->SendPacket(chatPacket);
	}

	void Arena::Update(float elapsedTime)
	{
		m_world.Update(elapsedTime);

		bool sendArenaStates = false;
		bool sendGhostStates = false;

		m_stateBroadcastAccumulator += elapsedTime;
		constexpr float stateBroadcastInterval = 1.f / 10.f;
		if (m_stateBroadcastAccumulator >= stateBroadcastInterval)
		{
			m_stateBroadcastAccumulator -= stateBroadcastInterval;

			sendArenaStates = true;
		}

		if constexpr (sendServerGhosts)
		{
			m_ghostBroadcastAccumulator += elapsedTime;

			constexpr float ghostBroadcastInterval = 1.f / 60.f;
			if (m_ghostBroadcastAccumulator >= ghostBroadcastInterval)
			{
				m_ghostBroadcastAccumulator -= ghostBroadcastInterval;

				sendGhostStates = true;
			}
		}

		if (sendArenaStates || sendGhostStates)
		{
			m_arenaStatePacket.serverTime = m_app->GetAppTime();
			m_arenaStatePacket.spaceships.clear();
			for (const Ndk::EntityHandle& spaceship : m_spaceships)
			{
				Ndk::NodeComponent& spaceshipNode = spaceship->GetComponent<Ndk::NodeComponent>();

				Packets::ArenaState::Spaceship spaceshipData;
				spaceshipData.id       = spaceship->GetId();
				spaceshipData.position = spaceshipNode.GetPosition();
				spaceshipData.rotation = spaceshipNode.GetRotation();

				m_arenaStatePacket.spaceships.emplace_back(std::move(spaceshipData));
			}
		}

		if (sendArenaStates)
		{
			for (auto& pair : m_players)
			{
				m_arenaStatePacket.lastProcessedInputTime = pair.first->GetLastInputProcessedTime();

				pair.first->SendPacket(m_arenaStatePacket);
			}
		}

		if (sendGhostStates)
		{
			// Broadcast arena state over network, for testing purposes
			Nz::NetPacket debugState(1);
			Packets::Serialize(debugState, m_arenaStatePacket);

			Nz::IpAddress debugAddress = Nz::IpAddress::BroadcastIpV4;
			debugAddress.SetPort(2050);

			m_debugSocket.SendPacket(debugAddress, debugState);
		}
	}

	void Arena::HandlePlayerLeave(Player* player)
	{
		assert(m_players.find(player) != m_players.end());

		m_players.erase(player);
	}

	void Arena::HandlePlayerJoin(Player* player)
	{
		assert(m_players.find(player) == m_players.end());

		for (auto& pair : m_players)
		{
			if (!pair.second)
				continue;

			Ndk::NodeComponent& spaceshipNode = pair.second->GetComponent<Ndk::NodeComponent>();

			Packets::CreateSpaceship createSpaceship;
			createSpaceship.id = pair.second->GetId();
			createSpaceship.position = spaceshipNode.GetPosition();
			createSpaceship.rotation = spaceshipNode.GetRotation();
			createSpaceship.name = pair.first->GetName();

			player->SendPacket(createSpaceship);
		}

		m_players[player] = Ndk::EntityHandle::InvalidHandle;
	}

	void Arena::OnSpaceshipDestroy(Ndk::Entity* spaceship)
	{
		Packets::DeleteSpaceship deletePacket;
		deletePacket.id = spaceship->GetId();

		for (auto& pair : m_players)
			pair.first->SendPacket(deletePacket);
	}
}
