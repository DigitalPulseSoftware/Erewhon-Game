// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Player.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent3D.hpp>
#include <Server/Arena.hpp>
#include <Server/ServerApplication.hpp>
#include <Server/Components/InputComponent.hpp>

namespace ewn
{
	Player::Player(ServerApplication* app, std::size_t peerId, NetworkReactor& reactor, const ServerCommandStore& commandStore) :
	m_arena(nullptr),
	m_app(app),
	m_networkReactor(reactor),
	m_commandStore(commandStore),
	m_peerId(peerId),
	m_lastInputTime(0),
	m_authenticated(false)
	{
	}

	Player::~Player()
	{
		if (m_arena)
			m_arena->HandlePlayerLeave(this);
	}

	void Player::Authenticate(std::string login)
	{
		m_login = std::move(login);
		m_authenticated = true;
	}

	Nz::UInt64 Player::GetLastInputProcessedTime() const
	{
		if (m_spaceship)
		{
			auto& controlComponent = m_spaceship->GetComponent<InputComponent>();
			return controlComponent.GetLastInputTime();
		}

		return 0;
	}

	void Player::MoveToArena(Arena* arena)
	{
		assert(m_arena != arena);

		if (m_arena)
			m_arena->HandlePlayerLeave(this);

		m_arena = arena;
		if (m_arena)
			m_arena->HandlePlayerJoin(this);

		m_spaceship = m_arena->CreatePlayerSpaceship(this);

		// Control packet
		Packets::ControlEntity controlPacket;
		controlPacket.id = m_spaceship->GetId();

		SendPacket(controlPacket);
	}

	void Player::Shoot()
	{
		if (m_app->GetAppTime() - m_lastShootTime < 500)
			return;

		auto& spaceshipNode = m_spaceship->GetComponent<Ndk::NodeComponent>();

		m_arena->CreateProjectile(this, m_spaceship, spaceshipNode.GetPosition() + spaceshipNode.GetForward() * 12.f, spaceshipNode.GetRotation());

		m_lastShootTime = m_app->GetAppTime();
	}

	void Player::UpdateInput(Nz::UInt64 lastInputTime, Nz::Vector3f movement, Nz::Vector3f rotation)
	{
		//TODO: Check input time consistency and possibly kick player
		if (lastInputTime <= m_lastInputTime)
			return;

		m_lastInputTime = lastInputTime;

		if (!m_spaceship)
			return;

		if (!std::isfinite(movement.x) ||
			!std::isfinite(movement.y) ||
			!std::isfinite(movement.z))
		{
			std::cout << "Client #" << m_peerId << " (" << m_login << " has non-finite movement: " << movement << std::endl;
			return;
		}

		if (!std::isfinite(rotation.x) ||
			!std::isfinite(rotation.y) ||
			!std::isfinite(rotation.z))
		{
			std::cout << "Client #" << m_peerId << " (" << m_login << " has non-finite rotation: " << movement << std::endl;
			return;
		}

		// TODO: Set speed limit accordingly to spaceship data
		movement.x = Nz::Clamp(movement.x, -1.f, 1.f);
		movement.y = Nz::Clamp(movement.y, -1.f, 1.f);
		movement.z = Nz::Clamp(movement.z, -1.f, 1.f);

		rotation.x = Nz::Clamp(rotation.x, -1.f, 1.f);
		rotation.y = Nz::Clamp(rotation.y, -1.f, 1.f);
		rotation.z = Nz::Clamp(rotation.z, -1.f, 1.f);

		auto& controlComponent = m_spaceship->GetComponent<InputComponent>();
		controlComponent.PushInput(lastInputTime, movement, rotation);
	}
}
