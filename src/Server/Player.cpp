// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Player.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent3D.hpp>
#include <Server/Arena.hpp>
#include <Server/ServerApplication.hpp>
#include <Server/Components/PlayerControlledComponent.hpp>

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
			auto& controlComponent = m_spaceship->GetComponent<PlayerControlledComponent>();
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

	void Player::UpdateInput(Nz::UInt64 lastInputTime, Nz::Vector3f direction, Nz::Vector3f rotation)
	{
		//TODO: Check input time consistency and possibly kick player
		if (lastInputTime <= m_lastInputTime)
			return;

		m_lastInputTime = lastInputTime;

		if (!m_spaceship)
			return;

		if (!std::isfinite(direction.x) ||
			!std::isfinite(direction.y) ||
			!std::isfinite(direction.z))
		{
			std::cout << "Client #" << m_peerId << " (" << m_login << " has non-finite direction: " << direction << std::endl;
			return;
		}

		if (!std::isfinite(rotation.x) ||
			!std::isfinite(rotation.y) ||
			!std::isfinite(rotation.z))
		{
			std::cout << "Client #" << m_peerId << " (" << m_login << " has non-finite rotation: " << direction << std::endl;
			return;
		}

		// TODO: Set speed limit accordingly to spaceship data
		direction.x = Nz::Clamp(direction.x, -50.f, 50.f);
		direction.y = Nz::Clamp(direction.y, -50.f, 50.f);
		direction.z = Nz::Clamp(direction.z, -50.f, 50.f);

		rotation.x = Nz::Clamp(rotation.x, -200.f, 200.f);
		rotation.y = Nz::Clamp(rotation.y, -200.f, 200.f);
		rotation.z = Nz::Clamp(rotation.z, -200.f, 200.f);

		PlayerControlledComponent& controlComponent = m_spaceship->GetComponent<PlayerControlledComponent>();
		controlComponent.PushInput(lastInputTime, direction, rotation);
	}
}
