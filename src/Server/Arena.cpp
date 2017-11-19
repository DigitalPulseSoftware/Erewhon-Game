// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Arena.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <Server/Player.hpp>
#include <Server/Components/PlayerControlledComponent.hpp>
#include <Server/Systems/SpaceshipSystem.hpp>
#include <cassert>

namespace ewn
{
	Arena::Arena()
	{
		m_world.AddSystem<SpaceshipSystem>();
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

	void Arena::Update(float elapsedTime)
	{
		m_world.Update(elapsedTime);

		if (m_stateClock.GetSeconds() > 1.f / 30.f)
		{
			m_stateClock.Restart();

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

			for (auto& pair : m_players)
				pair.first->SendPacket(m_arenaStatePacket);
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
