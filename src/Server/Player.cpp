// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Player.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent3D.hpp>
#include <Server/Arena.hpp>
#include <Server/ServerApplication.hpp>
#include <Server/Components/InputComponent.hpp>
#include <Server/Components/ScriptComponent.hpp>

namespace ewn
{
	Player::Player(ServerApplication* app, std::size_t peerId, NetworkReactor& reactor, const ServerCommandStore& commandStore) :
	m_arena(nullptr),
	m_app(app),
	m_networkReactor(reactor),
	m_commandStore(commandStore),
	m_peerId(peerId),
	m_databaseId(0),
	m_lastInputTime(0),
	m_authenticated(false)
	{
	}

	Player::~Player()
	{
		if (m_arena)
			m_arena->HandlePlayerLeave(this);
	}

	void Player::Authenticate(Nz::UInt32 dbId, std::function<void(Player*, bool succeeded)> authenticationCallback)
	{
		m_databaseId = dbId;

		m_app->GetGlobalDatabase().ExecuteQuery("LoadAccount", { Nz::Int32(dbId) }, [app = m_app, ply = CreateHandle(), cb = std::move(authenticationCallback)](DatabaseResult& result)
		{
			if (!ply)
				return;

			if (!result.IsValid())
			{
				std::cerr << "LoadAccount failed for player #" << ply->GetDatabaseId() << ": " << result.GetLastErrorMessage();

				cb(ply, false);
			}
			else if (result.GetRowCount() == 0)
			{
				std::cerr << "LoadAccount failed for player #" << ply->GetDatabaseId() << ": No account found";

				cb(ply, false);
			}
			else
			{
				std::string login = std::get<std::string>(result.GetValue(0, 0));
				std::string displayName = std::get<std::string>(result.GetValue(1, 0));

				ply->OnAuthenticated(std::move(login), std::move(displayName));

				cb(ply, true);

				app->GetGlobalDatabase().ExecuteQuery("UpdateLastLoginDate", { Nz::Int32(ply->GetDatabaseId()) }, [dbId = ply->GetDatabaseId()](DatabaseResult& result)
				{
					if (!result.IsValid() || result.GetAffectedRowCount() == 0)
						std::cerr << "Failed to update last login date for player #" << dbId << ": " << result.GetLastErrorMessage() << std::endl;
				});
			}
		});
	}

	const Ndk::EntityHandle& Player::InstantiateOrGetBot()
	{
		if (!m_botEntity)
		{
			auto& spaceshipNode = m_spaceship->GetComponent<Ndk::NodeComponent>();

			m_botEntity = m_arena->CreateEntity("spaceship", "Bot (" + m_login + ')', this, spaceshipNode.GetPosition() + spaceshipNode.GetDown() * 10.f, spaceshipNode.GetRotation());
		}

		return m_botEntity;
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
		if (ServerApplication::GetAppTime() - m_lastShootTime < 500)
			return;

		auto& spaceshipNode = m_spaceship->GetComponent<Ndk::NodeComponent>();

		m_arena->CreateProjectile(this, m_spaceship, spaceshipNode.GetPosition() + spaceshipNode.GetForward() * 12.f, spaceshipNode.GetRotation());

		m_lastShootTime = ServerApplication::GetAppTime();
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

	void Player::OnAuthenticated(std::string login, std::string displayName)
	{
		m_displayName = std::move(displayName);
		m_login = std::move(login);

		m_authenticated = true;
	}
}
