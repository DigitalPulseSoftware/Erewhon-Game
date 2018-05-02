// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Player.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent3D.hpp>
#include <Server/Arena.hpp>
#include <Server/ServerApplication.hpp>
#include <Server/Components/InputComponent.hpp>
#include <Server/Components/PlayerControlledComponent.hpp>
#include <Server/Components/ScriptComponent.hpp>
#include <cassert>

namespace ewn
{
	Player::Player(ServerApplication* app, std::size_t peerId, std::size_t sessionId, NetworkReactor& reactor, const ServerCommandStore& commandStore) :
	m_arena(nullptr),
	m_app(app),
	m_networkReactor(reactor),
	m_commandStore(commandStore),
	m_peerId(peerId),
	m_sessionId(sessionId),
	m_permissionLevel(0),
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

	void Player::Authenticate(Nz::Int32 dbId, std::function<void(Player*, bool succeeded)> authenticationCallback)
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
				std::string login = std::get<std::string>(result.GetValue(0));
				std::string displayName = std::get<std::string>(result.GetValue(1));
				Nz::Int16 permissionLevel = std::get<Nz::Int16>(result.GetValue(2));
				if (permissionLevel < 0)
					permissionLevel = 0;

				ply->OnAuthenticated(std::move(login), std::move(displayName), static_cast<Nz::UInt16>(permissionLevel));

				cb(ply, true);

				app->GetGlobalDatabase().ExecuteQuery("UpdateLastLoginDate", { Nz::Int32(ply->GetDatabaseId()) }, [dbId = ply->GetDatabaseId()](DatabaseResult& result)
				{
					if (!result.IsValid() || result.GetAffectedRowCount() == 0)
						std::cerr << "Failed to update last login date for player #" << dbId << ": " << result.GetLastErrorMessage() << std::endl;
				});
			}
		});
	}

	const Ndk::EntityHandle& Player::InstantiateBot(const std::string& name, std::size_t spaceshipHullId, Nz::Vector3f positionOffset)
	{
		constexpr std::size_t MaxBots = 10;

		Nz::Vector3f position;
		Nz::Quaternionf rotation;
		if (m_controlledEntity->HasComponent<Ndk::NodeComponent>())
		{
			auto& spaceshipNode = m_controlledEntity->GetComponent<Ndk::NodeComponent>();
			position = spaceshipNode.GetPosition() + spaceshipNode.GetDown() * 10.f;
			rotation = spaceshipNode.GetRotation();
		}
		else
		{
			position = Nz::Vector3f::Zero();
			rotation = Nz::Quaternionf::Identity();
		}

		position += positionOffset;

		if (m_botEntities.size() >= MaxBots)
			m_botEntities.erase(m_botEntities.begin());

		m_botEntities.emplace_back(m_arena->CreateSpaceship(name + " bot (" + m_login + ')', this, spaceshipHullId, position, rotation));

		return m_botEntities.back();
	}

	Nz::UInt64 Player::GetLastInputProcessedTime() const
	{
		if (m_controlledEntity)
		{
			auto& controlComponent = m_controlledEntity->GetComponent<InputComponent>();
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
	}

	void Player::PrintMessage(std::string chatMessage)
	{
		Packets::ChatMessage chatPacket;
		chatPacket.message = std::move(chatMessage);

		SendPacket(chatPacket);
	}

	void Player::Shoot()
	{
		if (ServerApplication::GetAppTime() - m_lastShootTime < 500)
			return;

		m_lastShootTime = ServerApplication::GetAppTime();

		auto& spaceshipNode = m_controlledEntity->GetComponent<Ndk::NodeComponent>();

		m_arena->CreatePlasmaProjectile(this, m_controlledEntity, spaceshipNode.GetPosition() + spaceshipNode.GetForward() * 12.f, spaceshipNode.GetRotation());

		Packets::PlaySound playSound;
		playSound.position = spaceshipNode.GetPosition();
		playSound.soundId = 0;

		m_arena->BroadcastPacket(playSound, this);
	}

	void Player::UpdateControlledEntity(const Ndk::EntityHandle& entity)
	{
		if (m_controlledEntity != entity)
		{
			assert(!entity || entity->HasComponent<PlayerControlledComponent>());

			m_controlledEntity = entity;

			// Control packet
			Packets::ControlEntity controlPacket;
			controlPacket.id = (m_controlledEntity) ? m_controlledEntity->GetId() : 0;
			SendPacket(controlPacket);
		}
	}

	void Player::UpdateInput(Nz::UInt64 lastInputTime, Nz::Vector3f movement, Nz::Vector3f rotation)
	{
		//TODO: Check input time consistency and possibly kick player
		if (lastInputTime <= m_lastInputTime)
			return;

		m_lastInputTime = lastInputTime;

		if (!m_controlledEntity)
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

		auto& controlComponent = m_controlledEntity->GetComponent<InputComponent>();
		controlComponent.PushInput(lastInputTime, movement, rotation);
	}

	void Player::UpdatePermissionLevel(Nz::UInt16 permissionLevel, std::function<void(bool updateSucceeded)> databaseCallback)
	{
		assert(m_authenticated);

		m_permissionLevel = permissionLevel;
		m_app->GetGlobalDatabase().ExecuteQuery("UpdatePermissionLevel", { Nz::Int32(m_databaseId), Nz::Int16(permissionLevel) }, [cb = std::move(databaseCallback)](DatabaseResult& result)
		{
			if (!result.IsValid())
				std::cerr << "Failed to update permission level: " << result.GetLastErrorMessage() << std::endl;
			else if (result.GetAffectedRowCount() == 0)
				std::cerr << "Failed to update permission level: player not found" << std::endl;

			if (cb)
				cb(result.IsValid() && result.GetAffectedRowCount() > 0);
		});
	}

	void Player::OnAuthenticated(std::string login, std::string displayName, Nz::UInt16 permissionLevel)
	{
		m_displayName = std::move(displayName);
		m_login = std::move(login);
		m_permissionLevel = permissionLevel;

		m_authenticated = true;
	}
}
