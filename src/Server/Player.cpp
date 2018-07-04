// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Player.hpp>
#include <Nazara/Core/StackArray.hpp>
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
	Player::Player(ServerApplication* app) :
	m_arena(nullptr),
	m_session(nullptr),
	m_app(app),
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

	bool Player::CanShoot() const
	{
		return ServerApplication::GetAppTime() - m_lastShootTime >= 500;
	}

	void Player::ClearControlledEntity()
	{
		if (m_controlledEntity)
		{
			m_controlledEntity->RemoveComponent<PlayerControlledComponent>();
			m_controlledEntity.Reset();
		}
	}

	void Player::CreateSpaceship(std::string name, std::string code, std::size_t hullId, std::vector<std::size_t> modules, std::function<void(Player*, bool succeded)> creationCallback)
{
		ServerApplication* app = m_app;

		DatabaseTransaction trans;
		trans.AppendPreparedStatement("CreateSpaceship", { GetDatabaseId(), std::move(name), std::move(code), Nz::Int32(hullId) }, [spaceshipModules = std::move(modules)](DatabaseTransaction& transaction, DatabaseResult result)
		{
			if (!result)
				return result;

			Nz::Int32 spaceshipId = std::get<Nz::Int32>(result.GetValue(0));

			Nz::StackArray<Nz::Int32> moduleIds = NazaraStackArrayNoInit(Nz::Int32, spaceshipModules.size());
			for (std::size_t i = 0; i < spaceshipModules.size(); ++i)
				moduleIds[i] = static_cast<Nz::Int32>(spaceshipModules[i]);

			// Warning: AppendPreparedStatement may free our lambda memory, meaning our captured variables are no longer valid, which is why we have to store the id on the function stack
			// Do not use data from here

			for (Nz::Int32 moduleId : moduleIds)
				transaction.AppendPreparedStatement("AddSpaceshipModule", { spaceshipId, moduleId });

			return result;
		});

		app->GetGlobalDatabase().ExecuteTransaction(std::move(trans), [app, sessionId = GetSessionId(), cb = std::move(creationCallback)](bool transactionSucceeded, std::vector<DatabaseResult>& queryResults)
		{
			if (!transactionSucceeded)
				std::cerr << "Create spaceship transaction failed: " << queryResults.back().GetLastErrorMessage() << std::endl;

			cb(app->GetPlayerBySession(sessionId), transactionSucceeded);
		});
	}

	void Player::GetFleetData(const std::string& fleetName, std::function<void(bool found, const FleetData& fleet)> callback, SpaceshipQueryInfoFlags infoFlags)
	{
		m_app->GetGlobalDatabase().ExecuteQuery("FindFleetByOwnerIdAndName", { GetDatabaseId(), fleetName }, [app = m_app, infoFlags, fleetName, cb = std::move(callback), sessionId = GetSessionId()](DatabaseResult& result)
		{
			if (!result)
			{
				cb(false, FleetData());

				std::cerr << "FindFleetByOwnerIdAndName failed: " << result.GetLastErrorMessage() << std::endl;
				return;
			}

			Player* ply = app->GetPlayerBySession(sessionId);
			if (!ply)
				return;

			if (result.GetRowCount() == 0)
			{
				cb(false, FleetData());
				return;
			}

			Nz::Int32 fleetId = std::get<Nz::Int32>(result.GetValue(0));

			app->GetGlobalDatabase().ExecuteQuery("FindFleetSpaceshipsByFleetId", { fleetId }, [app, infoFlags, fleetCallback = std::move(cb), fleetId, fleetName, sessionId](DatabaseResult& result)
			{
				if (!result)
				{
					fleetCallback(false, FleetData());

					std::cerr << "FindFleetSpaceshipByFleetId failed: " << result.GetLastErrorMessage() << std::endl;
					return;
				}

				Player* ply = app->GetPlayerBySession(sessionId);
				if (!ply)
					return;

				std::size_t rowCount = result.GetRowCount();
				if (rowCount == 0)
				{
					fleetCallback(false, FleetData());
					return;
				}

				Nz::Vector3f pos = Nz::Vector3f::Zero();

				ewn::DatabaseTransaction trans;

				FleetData pendingFleetData;
				pendingFleetData.fleetId = static_cast<std::size_t>(fleetId);
				pendingFleetData.fleetName = std::move(fleetName);
				pendingFleetData.spaceships.reserve(rowCount);

				for (std::size_t i = 0; i < rowCount; ++i)
				{
					Nz::Int32 spaceshipId = std::get<Nz::Int32>(result.GetValue(0, i));
					float posX = std::get<float>(result.GetValue(1, i));
					float posY = std::get<float>(result.GetValue(2, i));
					float posZ = std::get<float>(result.GetValue(3, i));

					auto& spaceshipData = pendingFleetData.spaceships.emplace_back();
					spaceshipData.position.Set(posX, posY, posZ);

					auto it = std::find_if(pendingFleetData.spaceshipTypes.begin(), pendingFleetData.spaceshipTypes.end(), [&](const auto& spaceshipTypeData)
					{
						return spaceshipTypeData.spaceshipId == spaceshipId;
					});

					if (it == pendingFleetData.spaceshipTypes.end())
					{
						spaceshipData.spaceshipType = pendingFleetData.spaceshipTypes.size();

						auto& spaceshipTypeData = pendingFleetData.spaceshipTypes.emplace_back();
						spaceshipTypeData.spaceshipId = spaceshipId;

						// New spaceship type, add it to request list
						trans.AppendPreparedStatement("FindSpaceshipById", { spaceshipId });

						if (infoFlags & SpaceshipQueryInfo::Modules)
							trans.AppendPreparedStatement("FindSpaceshipModulesBySpaceshipId", { spaceshipId });
					}
					else
						spaceshipData.spaceshipType = std::distance(pendingFleetData.spaceshipTypes.begin(), it);
				}

				app->GetGlobalDatabase().ExecuteTransaction(std::move(trans), [app, infoFlags, cb = std::move(fleetCallback), fleetData = std::move(pendingFleetData)](bool transactionSucceeded, std::vector<ewn::DatabaseResult>& results) mutable
				{
					if (!transactionSucceeded)
					{
						cb(false, FleetData());
						return;
					}

					std::size_t resultIndex = 1; //< Because of begin
					for (auto& spaceshipTypeData : fleetData.spaceshipTypes)
					{
						ewn::DatabaseResult& spaceshipResult = results[resultIndex];
						
						spaceshipTypeData.hullId = static_cast<std::size_t>(std::get<Nz::Int32>(spaceshipResult.GetValue(2)));
						spaceshipTypeData.collisionMeshId = app->GetSpaceshipHullStore().GetEntryCollisionMeshId(spaceshipTypeData.hullId);
						spaceshipTypeData.dimensions = app->GetCollisionMeshStore().GetEntryDimensions(spaceshipTypeData.collisionMeshId);

						if (infoFlags & SpaceshipQueryInfo::Code)
							spaceshipTypeData.script = std::get<std::string>(spaceshipResult.GetValue(1));

						if (infoFlags & SpaceshipQueryInfo::Name)
							spaceshipTypeData.name = std::get<std::string>(spaceshipResult.GetValue(0));

						if (infoFlags & SpaceshipQueryInfo::Modules)
						{
							ewn::DatabaseResult& moduleResult = results[resultIndex + 1];
							std::size_t moduleCount = moduleResult.GetRowCount();
							spaceshipTypeData.modules.reserve(moduleCount);
							for (std::size_t j = 0; j < moduleCount; ++j)
								spaceshipTypeData.modules.push_back(static_cast<std::size_t>(std::get<Nz::Int32>(moduleResult.GetValue(0, j))));

							resultIndex++;
						}

						resultIndex++;
					}

					cb(true, fleetData);
				});
			});
		});
	}

	const Ndk::EntityHandle& Player::InstantiateBot(const std::string& name, std::size_t spaceshipHullId, Nz::Vector3f positionOffset)
	{
		constexpr std::size_t MaxBots = 10;

		Nz::Vector3f position;
		Nz::Quaternionf rotation;
		if (m_controlledEntity && m_controlledEntity->HasComponent<Ndk::NodeComponent>())
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
		if (!m_controlledEntity)
			return;

		if (!CanShoot())
		{
			if (!std::holds_alternative<NoAction>(m_pendingAction))
				return;

			m_pendingAction.emplace<ShootAction>();
			return;
		}

		m_lastShootTime = ServerApplication::GetAppTime();

		auto& spaceshipNode = m_controlledEntity->GetComponent<Ndk::NodeComponent>();

		m_arena->CreatePlasmaProjectile(this, m_controlledEntity, spaceshipNode.GetPosition() + spaceshipNode.GetForward() * 12.f, spaceshipNode.GetRotation());

		Packets::PlaySound playSound;
		playSound.position = spaceshipNode.GetPosition();
		playSound.soundId = 0;

		m_arena->BroadcastPacket(playSound, this);
	}

	void Player::Update(float elapsedTime)
	{
		if (!std::holds_alternative<NoAction>(m_pendingAction))
		{
			bool hasFinished = std::visit([&](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, ShootAction>)
				{
					if (CanShoot())
					{
						Shoot();
						return true;
					}
					else
						return false;
				}
				else if constexpr (std::is_same_v<T, NoAction>)
				{
					// Shouldn't happen
					assert(false);
					return false;
				}
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

			}, m_pendingAction);

			if (hasFinished)
				m_pendingAction.emplace<NoAction>();
		}
	}

	void Player::UpdateControlledEntity(const Ndk::EntityHandle& entity)
	{
		if (m_controlledEntity != entity)
		{
			assert(!entity || !entity->HasComponent<PlayerControlledComponent>());

			ClearControlledEntity();

			m_controlledEntity = entity;
			if (m_controlledEntity)
				m_controlledEntity->AddComponent<PlayerControlledComponent>(this);

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
			std::cout << "Client #" << GetSessionId() << " (" << m_login << " has non-finite movement: " << movement << std::endl;
			return;
		}

		if (!std::isfinite(rotation.x) ||
		    !std::isfinite(rotation.y) ||
		    !std::isfinite(rotation.z))
		{
			std::cout << "Client #" << GetSessionId() << " (" << m_login << " has non-finite rotation: " << movement << std::endl;
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
		if (lastInputTime <= controlComponent.GetLastInputTime())
			return; //< FIXME?

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

	void Player::UpdateSession(ClientSession* session)
	{
		m_session = session;
	}

	void Player::OnAuthenticated(std::string login, std::string displayName, Nz::UInt16 permissionLevel)
	{
		m_displayName = std::move(displayName);
		m_login = std::move(login);
		m_permissionLevel = permissionLevel;

		m_authenticated = true;
	}
}
