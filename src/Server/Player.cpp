// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Player.hpp>
#include <Server/Arena.hpp>
#include <Server/Components/PlayerControlledComponent.hpp>

namespace ewn
{
	Player::Player(std::size_t peerId, NetworkReactor& reactor, const ServerCommandStore& commandStore) :
	m_commandStore(commandStore),
	m_arena(nullptr),
	m_networkReactor(reactor),
	m_peerId(peerId)
	{
	}

	void Player::SetArena(Arena* arena)
	{
		if (m_arena)
			m_arena->HandlePlayerLeave(this);

		m_arena = arena;
		if (m_arena)
			m_arena->HandlePlayerJoin(this);

		m_spaceship = m_arena->CreatePlayerSpaceship(this);
	}

	void Player::UpdateInput(const Nz::Vector3f& direction, const Nz::Vector3f& rotation)
	{
		if (!m_spaceship)
			return;

		PlayerControlledComponent& controlComponent = m_spaceship->GetComponent<PlayerControlledComponent>();
		controlComponent.Update(direction, rotation);
	}
}
