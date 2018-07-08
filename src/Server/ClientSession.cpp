// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ClientSession.hpp>
#include <Nazara/Core/StackArray.hpp>
#include <Shared/SecureRandomGenerator.hpp>
#include <Server/Components/OwnerComponent.hpp>
#include <Server/Components/ScriptComponent.hpp>
#include <Server/Player.hpp>
#include <Server/ServerApplication.hpp>
#include <argon2/argon2.h>
#include <bitset>
#include <cassert>
#include <cctype>
#include <iostream>
#include <regex>

namespace ewn
{
	ClientSession::ClientSession(ServerApplication* app, std::size_t sessionId, std::size_t peerId, std::shared_ptr<Player> player, NetworkReactor& reactor, const ServerCommandStore& commandStore) :
	m_player(std::move(player)),
	m_peerId(peerId),
	m_sessionId(sessionId),
	m_app(app),
	m_networkReactor(reactor),
	m_commandStore(commandStore)
	{
	}

	void ClientSession::HandleControlEntity(const Packets::ControlEntity& data)
	{
		Player* player = GetPlayer();
		if (!player->IsAuthenticated())
			return;

		if (Arena* arena = player->GetArena())
		{
			if (data.id != 0)
			{
				Ndk::EntityId entityId = static_cast<Ndk::EntityId>(data.id);
				if (!arena->IsEntityIdValid(entityId))
				{
					std::cerr << "Client #" << m_peerId << " tried to control invalid entity #" << entityId << std::endl;
					return;
				}

				const Ndk::EntityHandle& entity = arena->GetEntity(entityId);
				if (!entity->HasComponent<OwnerComponent>() || entity->GetComponent<OwnerComponent>().GetOwner() != player)
				{
					std::cerr << "Client #" << m_peerId << " tried to control entity #" << entityId << " which doesn't belong to them" << std::endl;
					return;
				}

				player->UpdateControlledEntity(entity);
			}
			else
				player->UpdateControlledEntity(Ndk::EntityHandle::InvalidHandle);
		}
	}

	void ClientSession::HandleCreateFleet(const Packets::CreateFleet& data)
	{
		Player* player = GetPlayer();
		if (!player->IsAuthenticated())
			return;

		if (data.fleetName.empty())
			return;

		if (data.spaceships.empty())
			return;

		Nz::Bitset<> usedNames(data.spaceshipNames.size(), false);
		for (const auto& spaceship : data.spaceships)
		{
			if (spaceship.spaceshipNameId >= data.spaceshipNames.size())
				return;

			usedNames[spaceship.spaceshipNameId] = true;

			constexpr float gridSize = 30.f;
			constexpr float maxHeight = 10.f;

			if (spaceship.spaceshipPosition.x < -gridSize || spaceship.spaceshipPosition.x > gridSize ||
			    spaceship.spaceshipPosition.z < -gridSize || spaceship.spaceshipPosition.z > gridSize ||
			    spaceship.spaceshipPosition.y < -maxHeight || spaceship.spaceshipPosition.y > maxHeight)
				return;
		}

		// If any name is not used, this may be a forged packet to increase database requests
		if (!usedNames.TestAll())
			return;

		DatabaseTransaction nameTransaction;
		for (const std::string& spaceshipName : data.spaceshipNames)
			nameTransaction.AppendPreparedStatement("FindSpaceshipIdByOwnerIdAndName", { player->GetDatabaseId(), spaceshipName });

		m_app->GetGlobalDatabase().ExecuteTransaction(std::move(nameTransaction), [data, app = m_app, sessionId = GetSessionId()](bool success, std::vector<DatabaseResult>& results)
		{
			Player* ply = app->GetPlayerBySession(sessionId);
			if (!ply)
				return;

			if (!success)
			{
				std::cerr << "Fleet creation id first pass failed: " << results.back().GetLastErrorMessage() << std::endl;

				Packets::CreateFleetFailure fleetFailure;
				fleetFailure.reason = CreateFleetFailureReason::ServerError;

				ply->SendPacket(fleetFailure);
				return;
			}

			for (std::size_t i = 1; i < results.size() - 1; ++i)
			{
				if (results[i].GetRowCount() == 0)
				{
					Packets::CreateFleetFailure fleetFailure;
					fleetFailure.reason = CreateFleetFailureReason::ServerError;

					ply->SendPacket(fleetFailure);
					return;
				}
			}

			struct SpaceshipData
			{
				Nz::Int32 spaceshipId;
				Nz::Vector3f position;
			};

			std::vector<SpaceshipData> spaceshipData;
			for (std::size_t i = 0; i < data.spaceships.size(); ++i)
			{
				auto& spaceship = spaceshipData.emplace_back();
				spaceship.position = data.spaceships[i].spaceshipPosition;

				std::size_t nameId = data.spaceships[i].spaceshipNameId;
				spaceship.spaceshipId = std::get<Nz::Int32>(results[1 + nameId].GetValue(0));
			}

			DatabaseTransaction fleetTrans;
			fleetTrans.AppendPreparedStatement("CreateFleet", { ply->GetDatabaseId(), data.fleetName }, [data = std::move(spaceshipData)](DatabaseTransaction& transaction, DatabaseResult result)
			{
				if (!result)
					return result;

				auto spaceshipData = std::move(data);

				Nz::Int32 fleetId = std::get<Nz::Int32>(result.GetValue(0));

				for (const auto& spaceship : spaceshipData)
					transaction.AppendPreparedStatement("CreateFleetSpaceship", { fleetId, spaceship.spaceshipId, spaceship.position.x, spaceship.position.y, spaceship.position.z });

				return result;
			});

			app->GetGlobalDatabase().ExecuteTransaction(std::move(fleetTrans), [app, sessionId](bool success, std::vector<DatabaseResult>& results)
			{
				Player* ply = app->GetPlayerBySession(sessionId);
				if (!ply)
					return;

				if (!success)
				{
					Packets::CreateFleetFailure creationFailed;
					if (results.size() == 2) //< Begin + CreateFleet result
						creationFailed.reason = CreateFleetFailureReason::AlreadyExists;
					else
						creationFailed.reason = CreateFleetFailureReason::ServerError;

					ply->SendPacket(creationFailed);
					return;
				}

				ply->SendPacket(Packets::CreateFleetSuccess());
			});
		});
	}

	void ClientSession::HandleCreateSpaceship(const Packets::CreateSpaceship& data)
	{
		Player* player = GetPlayer();
		if (!player->IsAuthenticated())
			return;

		if (data.spaceshipName.empty())
			return;

		if (data.spaceshipCode.empty())
			return;

		if (data.modules.empty())
			return;

		if (!m_app->GetSpaceshipHullStore().IsEntryLoaded(data.hullId))
			return;

		const ModuleStore& moduleStore = m_app->GetModuleStore();

		std::bitset<static_cast<std::size_t>(ModuleType::Max) + 1> receivedModules;

		std::size_t maxModuleId = moduleStore.GetEntryCount() + 1;
		for (const auto& packetModule : data.modules)
		{
			if (packetModule.type > ModuleType::Max)
				return;

			if (packetModule.moduleId > maxModuleId)
				return;

			if (!moduleStore.IsEntryLoaded(packetModule.moduleId))
				return;

			if (moduleStore.GetEntryType(packetModule.moduleId) != packetModule.type)
				return;

			// Check if module was already sent
			std::size_t moduleBit = static_cast<std::size_t>(packetModule.type);
			if (receivedModules.test(moduleBit))
				return;

			receivedModules.set(moduleBit);
		}

		// Check if all required modules were received
		if (!receivedModules.all())
			return;

		std::vector<std::size_t> modules;
		modules.reserve(data.modules.size());

		for (const auto& packetModule : data.modules)
			modules.push_back(packetModule.moduleId);

		player->CreateSpaceship(data.spaceshipName, data.spaceshipCode, data.hullId, std::move(modules), [](Player* player, bool succeeded)
		{
			if (!player)
				return;

			if (!succeeded)
			{
				Packets::CreateSpaceshipFailure createFailure;
				createFailure.reason = CreateSpaceshipFailureReason::AlreadyExists;

				player->SendPacket(createFailure);
				return;
			}

			player->SendPacket(Packets::CreateSpaceshipSuccess{});
		});
	}

	void ClientSession::HandleDeleteFleet(const Packets::DeleteFleet & data)
	{
		Player* player = GetPlayer();
		if (!player->IsAuthenticated())
			return;

		m_app->GetGlobalDatabase().ExecuteStatement("DeleteFleet", { player->GetDatabaseId(), data.fleetName }, [app = m_app, sessionId = GetSessionId()](DatabaseResult& result)
		{
			Player* ply = app->GetPlayerBySession(sessionId);
			if (!ply)
				return;

			if (result.GetAffectedRowCount() == 0)
			{
				Packets::DeleteFleetFailure deleteFailure;
				deleteFailure.reason = DeleteFleetFailureReason::NotFound;

				ply->SendPacket(deleteFailure);
				return;
			}

			ply->SendPacket(Packets::DeleteFleetSuccess());
		});
	}

	void ClientSession::HandleDeleteSpaceship(const Packets::DeleteSpaceship & data)
	{
		Player* player = GetPlayer();
		if (!player->IsAuthenticated())
			return;

		Nz::Int32 playerDatabaseId = Nz::Int32(player->GetDatabaseId());

		DatabaseTransaction trans;
		trans.AppendPreparedStatement("CountSpaceshipByOwnerIdExceptName", { playerDatabaseId, data.spaceshipName }, [app = m_app, playerDatabaseId, sessionId = player->GetSessionId(), spaceshipName = data.spaceshipName](DatabaseTransaction& transaction, DatabaseResult result)->DatabaseResult
		{
			if (!result)
				return result;

			Nz::Int64 spaceshipCount = std::get<Nz::Int64>(result.GetValue(0));
			if (spaceshipCount == 0)
			{
				Player* ply = app->GetPlayerBySession(sessionId);
				if (!ply)
					return DatabaseResult{};

				Packets::DeleteSpaceshipFailure deleteFailure;
				deleteFailure.reason = DeleteSpaceshipFailureReason::MustHaveAtLeastOne;

				ply->SendPacket(deleteFailure);
				return DatabaseResult{};
			}

			transaction.AppendPreparedStatement("DeleteSpaceship", { playerDatabaseId, spaceshipName });
			return result;
		});

		m_app->GetGlobalDatabase().ExecuteTransaction(std::move(trans), [app = m_app, sessionId = GetSessionId()](bool transactionSucceeded, std::vector<DatabaseResult>& queryResults)
		{
			if (!transactionSucceeded)
			{
				// Check if we issued "DeleteSpaceship" statement, if not assume error has already been handled
				if (queryResults.size() < 4)
					return;

				std::cerr << "Delete spaceship transaction failed: " << queryResults.back().GetLastErrorMessage() << std::endl;

				Player* ply = app->GetPlayerBySession(sessionId);
				if (!ply)
					return;

				Packets::DeleteSpaceshipFailure deleteFailure;
				deleteFailure.reason = DeleteSpaceshipFailureReason::ServerError;

				ply->SendPacket(deleteFailure);
				return;
			}

			Player* ply = app->GetPlayerBySession(sessionId);
			if (!ply)
				return;

			assert(queryResults.size() >= 4);

			constexpr std::size_t DeleteSpaceshipResultIndex = 2;

			if (queryResults[DeleteSpaceshipResultIndex].GetAffectedRowCount() == 0)
			{
				Packets::DeleteSpaceshipFailure deleteFailure;
				deleteFailure.reason = DeleteSpaceshipFailureReason::NotFound;

				ply->SendPacket(deleteFailure);
				return;
			}

			ply->SendPacket(Packets::DeleteSpaceshipSuccess{});
		});
	}

	void ClientSession::HandleLoginSucceeded(Nz::Int32 databaseId, bool regenerateToken)
	{
		// Generate connection token
		SecureRandomGenerator gen;

		std::vector<Nz::UInt8> token;
		if (regenerateToken)
		{
			token.resize(64);
			if (!gen(token.data(), token.size()))
			{
				std::cerr << "SecureRandomGenerator failed" << std::endl;
				token.clear();
			}
		}

		m_app->RegisterCallback([app = m_app, sessionId = GetSessionId(), databaseId, connectionToken = std::move(token)]()
		{
			Player* ply = app->GetPlayerBySession(sessionId);
			if (!ply)
				return;

			ply->Authenticate(databaseId, [app, playerToken = std::move(connectionToken)](Player* player, bool loginSuccess)
			{
				if (loginSuccess)
				{
					if (!playerToken.empty())
					{
						std::string tokenAsString(128, ' ');
						for (std::size_t i = 0; i < playerToken.size(); ++i)
							std::sprintf(&tokenAsString[i * 2], "%02x", playerToken[i]);

						DatabaseTransaction dbTransaction;
						dbTransaction.AppendPreparedStatement("DeleteAccountTokenByAccountId", { player->GetDatabaseId() });
						dbTransaction.AppendPreparedStatement("CreateAccountToken", { player->GetDatabaseId(), tokenAsString });

						app->GetGlobalDatabase().ExecuteTransaction(std::move(dbTransaction), [app, packetToken = std::move(playerToken), sessionId = player->GetSessionId()](bool transactionSucceeded, std::vector<DatabaseResult>& queryResults)
						{
							Player* player = app->GetPlayerBySession(sessionId);
							if (!player)
								return;

							if (transactionSucceeded)
							{
								Packets::LoginSuccess loginSuccess;
								loginSuccess.connectionToken = std::move(packetToken);

								player->SendPacket(loginSuccess);
								std::cout << "Player #" << player->GetSession()->GetPeerId() << " authenticated as " << player->GetName() << " and regenerated a connection token" << std::endl;
							}
							else
							{
								std::cout << "Failed to save token: " << queryResults.back().GetLastErrorMessage() << std::endl;
								player->SendPacket(Packets::LoginSuccess());
								std::cout << "Player #" << player->GetSession()->GetPeerId() << " authenticated as " << player->GetName() << std::endl;
							}
						});
					}
					else
					{
						player->SendPacket(Packets::LoginSuccess());
						std::cout << "Player #" << player->GetSession()->GetPeerId() << " authenticated as " << player->GetName() << std::endl;
					}
				}
				else
				{
					std::cerr << "Failed to authenticate player #" << player->GetSession()->GetPeerId() << ": Database authentication failed" << std::endl;

					Packets::LoginFailure loginFailure;
					loginFailure.reason = LoginFailureReason::ServerError;

					player->SendPacket(loginFailure);
				}
			});
		});
	}

	void ClientSession::HandleLogin(const Packets::Login& data)
	{
		Player* player = GetPlayer();
		if (player->IsAuthenticated())
			return;

		if (data.login.empty() || data.login.size() > 20)
			return;

		Account_QueryConnectionInfoByLogin request;
		request.login = data.login;

		m_app->GetGlobalDatabase().ExecuteStatement(std::move(request),
		[app = m_app, sessionId = player->GetSessionId(), login = data.login, pwd = data.passwordHash, needToken = data.generateConnectionToken](DatabaseResult& result)
		{
			Player* ply = app->GetPlayerBySession(sessionId);
			if (!ply)
				return;

			if (!result.IsValid())
			{
				Packets::LoginFailure loginFailure;
				loginFailure.reason = LoginFailureReason::ServerError;

				ply->SendPacket(loginFailure);
				return;
			}

			if (result.GetRowCount() == 0)
			{
				std::cout << "Player #" << ply->GetSession()->GetPeerId() << " authentication as " << login << " failed: player not found" << std::endl;

				Packets::LoginFailure loginFailure;
				loginFailure.reason = LoginFailureReason::AccountNotFound;

				ply->SendPacket(loginFailure);
				return;
			}

			assert(result.GetRowCount() == 1);

			ConfigFile& config = app->GetConfig();
			const std::string& globalSalt = config.GetStringOption("Security.PasswordSalt");

			Account_QueryConnectionInfoByLogin::Result dbResult(result);

			int iCost = config.GetIntegerOption<int>("Security.Argon2.IterationCost");
			int mCost = config.GetIntegerOption<int>("Security.Argon2.MemoryCost");
			int tCost = config.GetIntegerOption<int>("Security.Argon2.ThreadCost");
			int hashLength = config.GetIntegerOption<int>("Security.HashLength");

			app->DispatchWork([app, salt = globalSalt + dbResult.salt, pass = std::move(pwd), dbPass = dbResult.password, id = dbResult.id, sessionId, login, iCost, mCost, tCost, hashLength, needToken]()
			{
				Nz::StackArray<uint8_t> output = NazaraStackArrayNoInit(uint8_t, hashLength);
				Nz::StackArray<char> outputHex = NazaraStackArrayNoInit(char, hashLength * 2 + 1);

				argon2_context context;
				std::memset(&context, 0, sizeof(argon2_context));

				context.out = output.data();
				context.outlen = uint32_t(hashLength);
				context.pwd = reinterpret_cast<uint8_t*>(const_cast<char*>(pass.data()));
				context.pwdlen = uint32_t(pass.size());
				context.salt = reinterpret_cast<uint8_t*>(const_cast<char*>(salt.data()));
				context.saltlen = uint32_t(salt.size());
				context.t_cost = iCost;
				context.m_cost = mCost;
				context.lanes = tCost;
				context.threads = tCost;
				context.flags = ARGON2_DEFAULT_FLAGS;
				context.version = ARGON2_VERSION_13;

				std::optional<LoginFailureReason> failure;
				int argon2Ret = argon2_ctx(&context, argon2_type::Argon2_id);
				if (argon2Ret == ARGON2_OK)
				{
					for (std::size_t i = 0; i < output.size(); ++i)
						std::sprintf(&outputHex[i * 2], "%02x", output[i]);

					// Protect against timing-attack
					assert(dbPass.size() == outputHex.size() - 1);

					int isDifferent = 0;
					for (std::size_t i = 0; i < dbPass.size(); ++i)
						isDifferent |= (outputHex[i] ^ dbPass[i]);

					if (isDifferent)
						failure = LoginFailureReason::PasswordMismatch;
				}
				else
					failure = LoginFailureReason::ServerError;

				if (!failure)
				{
					Player* ply = app->GetPlayerBySession(sessionId);
					if (!ply)
						return;

					ply->GetSession()->HandleLoginSucceeded(id, needToken);
				}
				else
				{
					app->RegisterCallback([app, sessionId, login, reason = failure.value(), argon2Ret]()
					{
						Player* ply = app->GetPlayerBySession(sessionId);
						if (!ply)
							return;

						Packets::LoginFailure loginFailure;
						loginFailure.reason = reason;

						ply->SendPacket(loginFailure);

						switch (reason)
						{
							case LoginFailureReason::PasswordMismatch:
								std::cout << "Player #" << ply->GetSession()->GetPeerId() << " authentication as " << login << " failed: password mismatch" << std::endl;
								break;

							case LoginFailureReason::ServerError:
								std::cout << "Player #" << ply->GetSession()->GetPeerId() << " authentication as " << login << " failed: argon2 failure (err: " << argon2Ret << ")" << std::endl;
								break;

							case LoginFailureReason::AccountNotFound:
							default:
								assert(false);
								break;
						}
					});
				}
			});
		});
	}

	void ClientSession::HandleLoginByToken(const Packets::LoginByToken& data)
	{
		Player* player = GetPlayer();
		if (player->IsAuthenticated())
			return;

		if (data.connectionToken.size() != 64)
			return;

		std::string tokenAsString(128, ' ');
		for (std::size_t i = 0; i < data.connectionToken.size(); ++i)
			std::sprintf(&tokenAsString[i * 2], "%02x", data.connectionToken[i]);

		DatabaseTransaction trans;
		trans.AppendPreparedStatement("FindAccountByToken", { tokenAsString }, [](DatabaseTransaction& transaction, DatabaseResult result) -> DatabaseResult
		{
			if (result && result.GetRowCount() != 0)
			{
				// Delete token after retrieving it

				Nz::Int32 dbId = std::get<Nz::Int32>(result.GetValue(0));

				transaction.AppendPreparedStatement("DeleteAccountTokenByAccountId", { dbId });
			}

			return result;
		});

		std::size_t accountResultId = trans.GetBeginResultIndex() + 1;
		m_app->GetGlobalDatabase().ExecuteTransaction(std::move(trans), [app = m_app, sessionId = player->GetSessionId(), accountResultId, generateNewToken = data.generateConnectionToken](bool transactionSucceeded, std::vector<DatabaseResult>& queryResults)
		{
			Player* ply = app->GetPlayerBySession(sessionId);
			if (!ply)
				return;

			if (!transactionSucceeded || queryResults[accountResultId].GetRowCount() == 0)
			{
				std::cout << "Player #" << ply->GetSession()->GetPeerId() << " authentication via token failed" << std::endl;

				Packets::LoginFailure loginFailure;
				loginFailure.reason = LoginFailureReason::InvalidToken;

				ply->SendPacket(loginFailure);
				return;
			}

			Nz::Int32 dbId = std::get<Nz::Int32>(queryResults[accountResultId].GetValue(0));
			ply->GetSession()->HandleLoginSucceeded(dbId, generateNewToken);
		});
	}

	void ClientSession::HandleLeaveArena(const Packets::LeaveArena& data)
	{
		Player* player = GetPlayer();
		if (!player->IsAuthenticated())
			return;

		if (player->GetArena())
			player->MoveToArena(nullptr);
	}

	void ClientSession::HandleJoinArena(const Packets::JoinArena& data)
	{
		Player* player = GetPlayer();
		if (!player->IsAuthenticated())
			return;

		if (data.arenaIndex > m_app->GetArenaCount())
			return;

		Arena* arena = m_app->GetArena(data.arenaIndex);
		if (player->GetArena() != arena)
			player->MoveToArena(arena);
	}

	void ClientSession::HandlePlayerChat(const Packets::PlayerChat& data)
	{
		Player* player = GetPlayer();
		if (!player->IsAuthenticated())
			return;

		if (data.text.empty())
			return;

		if (data.text[0] == '/')
		{
			std::string_view command = data.text;
			command.remove_prefix(1);

			std::optional<bool> result = m_app->GetChatCommandStore().ExecuteCommand(player, command);
			if (result)
			{
				if (!*result)
					player->PrintMessage("Error in parsing command, you may be missing parameters");

				return; // Don't show command if it was recognized
			}
		}

		if (Arena* arena = player->GetArena())
			arena->HandleChatMessage(player, data.text);
	}

	void ClientSession::HandlePlayerMovement(const Packets::PlayerMovement& data)
	{
		Player* player = GetPlayer();
		if (!player->IsAuthenticated())
			return;

		player->UpdateInput(data.inputTime, data.direction, data.rotation);
	}

	void ClientSession::HandlePlayerShoot(const Packets::PlayerShoot& data)
	{
		Player* player = GetPlayer();
		if (!player->IsAuthenticated())
			return;

		player->Shoot();
	}

	void ClientSession::HandleQueryArenaList(const Packets::QueryArenaList& data)
	{
		Player* player = GetPlayer();
		if (!player->IsAuthenticated())
			return;

		std::size_t arenaCount = m_app->GetArenaCount();

		Packets::ArenaList listPacket;
		listPacket.arenas.reserve(arenaCount);

		for (std::size_t i = 0; i < arenaCount; ++i)
		{
			auto& arenaData = listPacket.arenas.emplace_back();
			arenaData.arenaName = m_app->GetArena(i)->GetName();
		}

		player->SendPacket(listPacket);
	}

	void ClientSession::HandleQueryFleetInfo(const Packets::QueryFleetInfo& data)
	{
		Player* player = GetPlayer();
		if (!player->IsAuthenticated())
			return;

		player->GetFleetData(data.fleetName, [app = m_app, infoFlags = data.spaceshipInfo, sessionId = GetSessionId()](bool found, const Player::FleetData& fleet)
		{
			Player* player = app->GetPlayerBySession(sessionId);
			if (!player->IsAuthenticated())
				return;

			Packets::FleetInfo fleetInfo;

			if (!found)
			{
				player->SendPacket(fleetInfo);
				return;
			}

			fleetInfo.fleetName = fleet.fleetName;
			fleetInfo.spaceshipInfo = infoFlags;

			auto& collisionMeshStore = app->GetCollisionMeshStore();
			auto& moduleStore = app->GetModuleStore();
			auto& spaceshipHullStore = app->GetSpaceshipHullStore();
			auto& visualMeshStore = app->GetVisualMeshStore();

			for (const auto& type : fleet.spaceshipTypes)
			{
				std::size_t collisionMeshId = spaceshipHullStore.GetEntryCollisionMeshId(type.hullId);

				auto& typeData = fleetInfo.spaceshipTypes.emplace_back();
				typeData.dimensions = type.dimensions;
				typeData.scale = collisionMeshStore.GetEntryScale(collisionMeshId);

				if (infoFlags & SpaceshipQueryInfo::Code)
					typeData.script = type.script;

				if (infoFlags & SpaceshipQueryInfo::HullModelPath)
				{
					std::size_t visualMeshId = spaceshipHullStore.GetEntryVisualMeshId(type.hullId);
					typeData.hullModelPath = visualMeshStore.GetEntryFilePath(visualMeshId);
				}

				if (infoFlags & SpaceshipQueryInfo::Name)
					typeData.name = type.name;

				if (infoFlags & SpaceshipQueryInfo::Modules)
				{
					for (std::size_t moduleId : type.modules)
					{
						auto& moduleData = typeData.modules.emplace_back();
						moduleData.currentModule = static_cast<Nz::UInt32>(moduleId);
						moduleData.type = moduleStore.GetEntryType(moduleId);
					}
				}
			}

			for (const auto& spaceship : fleet.spaceships)
			{
				auto& spaceshipData = fleetInfo.spaceships.emplace_back();
				spaceshipData.position = spaceship.position;
				spaceshipData.spaceshipType = spaceship.spaceshipType;
			}

			player->SendPacket(fleetInfo);

		}, data.spaceshipInfo);
	}

	void ClientSession::HandleQueryFleetList(const Packets::QueryFleetList& data)
	{
		Player* player = GetPlayer();
		if (!player->IsAuthenticated())
			return;

		m_app->GetGlobalDatabase().ExecuteStatement("FindFleetsByOwnerId", { player->GetDatabaseId() }, [app = m_app, sessionId = GetSessionId()](DatabaseResult& result)
		{
			Player* ply = app->GetPlayerBySession(sessionId);
			if (!ply)
				return;

			Packets::FleetList fleetList;

			if (!result)
			{
				std::cerr << "FindFleetsByOwnerId failed: " << result.GetLastErrorMessage() << std::endl;
				ply->SendPacket(fleetList);
				return;
			}

			std::size_t fleetCount = result.GetRowCount();
			for (std::size_t i = 0; i < fleetCount; ++i)
			{
				auto& fleetData = fleetList.fleets.emplace_back();
				fleetData.name = std::get<std::string>(result.GetValue(1, i));
			}

			ply->SendPacket(fleetList);
		});
	}

	void ClientSession::HandleQueryHullList(const Packets::QueryHullList& data)
	{
		Player* player = GetPlayer();
		if (!player->IsAuthenticated())
			return;

		auto& spaceshipHullStore = m_app->GetSpaceshipHullStore();
		auto& stringStore = m_app->GetNetworkStringStore();
		auto& visualMeshStore = m_app->GetVisualMeshStore();

		Packets::HullList hullList;
		hullList.hulls.reserve(spaceshipHullStore.GetEntryCount());

		for (std::size_t i = 0; i < spaceshipHullStore.GetEntryCount(); ++i)
		{
			if (spaceshipHullStore.IsEntryLoaded(i))
			{
				std::size_t visualMeshId = spaceshipHullStore.GetEntryVisualMeshId(i);

				auto& hullInfo = hullList.hulls.emplace_back();
				hullInfo.description = spaceshipHullStore.GetEntryDescription(i);
				hullInfo.hullId = static_cast<Nz::UInt32>(i);
				hullInfo.hullModelPathId = stringStore.GetStringIndex(visualMeshStore.GetEntryFilePath(visualMeshId));
				hullInfo.name = spaceshipHullStore.GetEntryName(i);

				hullInfo.slots.reserve(spaceshipHullStore.GetEntrySlotCount(i));

				for (std::size_t j = 0; j < spaceshipHullStore.GetEntrySlotCount(i); ++j)
				{
					auto& slotInfo = hullInfo.slots.emplace_back();
					slotInfo.type = spaceshipHullStore.GetEntrySlotModuleType(i, j);
				}
			}
		}

		player->SendPacket(hullList);
	}

	void ClientSession::HandleQueryModuleList(const Packets::QueryModuleList& data)
	{
		Player* player = GetPlayer();
		if (!player->IsAuthenticated())
			return;

		auto& moduleStore = m_app->GetModuleStore();

		Packets::ModuleList moduleList;

		for (std::size_t i = 0; i < moduleStore.GetEntryCount(); ++i)
		{
			if (moduleStore.IsEntryLoaded(i))
			{
				ModuleType type = moduleStore.GetEntryType(i);

				auto it = std::find_if(moduleList.modules.begin(), moduleList.modules.end(), [type](const Packets::ModuleList::ModuleTypeInfo& typeInfo)
				{
					return typeInfo.type == type;
				});

				if (it == moduleList.modules.end())
				{
					auto& moduleTypeInfo = moduleList.modules.emplace_back();
					moduleTypeInfo.type = type;

					it = moduleList.modules.end() - 1;
				}

				auto& moduleTypeInfo = *it;
				auto& moduleInfo = moduleTypeInfo.availableModules.emplace_back();
				moduleInfo.moduleId = static_cast<Nz::UInt32>(i);
				moduleInfo.moduleName = moduleStore.GetEntryName(i);
			}
		}

		player->SendPacket(moduleList);
	}

	void ClientSession::HandleQuerySpaceshipInfo(const Packets::QuerySpaceshipInfo& data)
	{
		Player* player = GetPlayer();
		if (!player->IsAuthenticated())
			return;

		if (data.spaceshipName.empty())
			return;

		//TODO: SQL function
		m_app->GetGlobalDatabase().ExecuteStatement("FindSpaceshipByOwnerIdAndName", { player->GetDatabaseId(), data.spaceshipName }, [app = m_app, infoFlags = data.info, name = data.spaceshipName, sessionId = player->GetSessionId()](DatabaseResult& result)
		{
			Player* ply = app->GetPlayerBySession(sessionId);
			if (!ply)
				return; //< Player has disconnected, ignore

			if (!result)
			{
				std::cerr << "FindSpaceshipByOwnerIdAndName failed: " << result.GetLastErrorMessage() << std::endl;

				ply->SendPacket(Packets::SpaceshipInfo());
				return;
			}

			if (result.GetRowCount() == 0)
			{
				ply->SendPacket(Packets::SpaceshipInfo());
				return;
			}

			Nz::Int32 spaceshipId = std::get<Nz::Int32>(result.GetValue(0));
			std::string spaceshipCode = std::get<std::string>(result.GetValue(1));
			Nz::UInt32 spaceshipHullId = static_cast<Nz::UInt32>(std::get<Nz::Int32>(result.GetValue(2)));

			auto SendResponse = [=, code = std::move(spaceshipCode)](Player* ply, DatabaseResult& result)
			{
				auto& collisionMeshStore = app->GetCollisionMeshStore();
				auto& moduleStore = app->GetModuleStore();
				auto& spaceshipHullStore = app->GetSpaceshipHullStore();
				auto& visualMeshStore = app->GetVisualMeshStore();

				std::size_t visualMeshId = spaceshipHullStore.GetEntryVisualMeshId(spaceshipHullId);

				Packets::SpaceshipInfo spaceshipInfo;
				spaceshipInfo.info = infoFlags;

				std::size_t collisionMeshId = spaceshipHullStore.GetEntryCollisionMeshId(spaceshipHullId);
				spaceshipInfo.collisionBox = collisionMeshStore.GetEntryDimensions(collisionMeshId);
				spaceshipInfo.hullId = spaceshipHullId;
				spaceshipInfo.scale = collisionMeshStore.GetEntryScale(collisionMeshId);

				if (infoFlags & SpaceshipQueryInfo::Code)
					spaceshipInfo.code = std::move(code);

				if (infoFlags & SpaceshipQueryInfo::HullModelPath)
					spaceshipInfo.hullModelPath = visualMeshStore.GetEntryFilePath(visualMeshId);

				if (infoFlags & SpaceshipQueryInfo::Name)
					spaceshipInfo.spaceshipName = name;

				// Spaceship actual modules
				if (infoFlags & SpaceshipQueryInfo::Modules)
				{
					assert(result);

					std::size_t moduleCount = result.GetRowCount();

					spaceshipInfo.modules.reserve(moduleCount);
					for (std::size_t i = 0; i < moduleCount; ++i)
					{
						Nz::UInt32 moduleId = static_cast<Nz::UInt32>(std::get<Nz::Int32>(result.GetValue(0, i)));

						auto& moduleInfo = spaceshipInfo.modules.emplace_back();
						moduleInfo.type = moduleStore.GetEntryType(moduleId);
						moduleInfo.currentModule = moduleId;
					}
				}

				ply->SendPacket(spaceshipInfo);
			};

			if (infoFlags & SpaceshipQueryInfo::Modules)
			{
				app->GetGlobalDatabase().ExecuteStatement("FindSpaceshipModulesBySpaceshipId", { spaceshipId }, [app, sessionId, cb = std::move(SendResponse)](DatabaseResult& result)
				{
					Player* ply = app->GetPlayerBySession(sessionId);
					if (!ply)
						return; //< Player has disconnected, ignore

					if (!result.IsValid())
					{
						ply->SendPacket(Packets::SpaceshipInfo());
						return;
					}

					cb(ply, result);
				});
			}
			else
			{
				DatabaseResult dummy;
				SendResponse(ply, dummy);
			}
		});
	}

	void ClientSession::HandleQuerySpaceshipList(const Packets::QuerySpaceshipList& /*data*/)
	{
		Player* player = GetPlayer();
		if (!player->IsAuthenticated())
			return;

		m_app->GetGlobalDatabase().ExecuteStatement("FindSpaceshipsByOwnerId", { Nz::Int32(player->GetDatabaseId()) }, [app = m_app, sessionId = player->GetSessionId()](DatabaseResult& result)
		{
			Player* ply = app->GetPlayerBySession(sessionId);
			if (!ply)
				return; //< Player has disconnected, ignore

			if (result.IsValid())
			{
				std::size_t rowCount = result.GetRowCount();
				if (rowCount > 0)
				{
					Packets::SpaceshipList spaceshipList;

					spaceshipList.spaceships.resize(rowCount);
					for (std::size_t i = 0; i < rowCount; ++i)
					{
						auto& spaceship = spaceshipList.spaceships[i];
						spaceship.name = std::get<std::string>(result.GetValue(1, i));
					}

					ply->SendPacket(spaceshipList);
				}
				else
				{
					// No spaceship, create the default one
					const auto& defaultSpaceshipData = app->GetDefaultSpaceshipData();

					ply->CreateSpaceship(defaultSpaceshipData.name, defaultSpaceshipData.code, defaultSpaceshipData.hullId, defaultSpaceshipData.moduleIds, [app](Player* player, bool succeeded)
					{
						if (!player)
							return;

						Packets::SpaceshipList spaceshipList;
						if (succeeded)
						{
							// Fill list manually
							spaceshipList.spaceships.resize(1);
							spaceshipList.spaceships.back().name = app->GetDefaultSpaceshipData().name;
						}

						player->SendPacket(spaceshipList);
					});
				}
			}
			else
			{
				std::cerr << "FindSpaceshipsByOwnerId failed:" << result.GetLastErrorMessage() << std::endl;
				ply->SendPacket(Packets::SpaceshipList{});
			}
		});
	}

	void ClientSession::HandleRegister(const Packets::Register& data)
	{
		Player* player = GetPlayer();
		if (player->IsAuthenticated())
			return;

		if (data.login.empty() || data.login.size() > 20)
			return;

		if (data.email.empty() || data.email.size() > 40)
			return;

		if (data.passwordHash.empty() || data.passwordHash.size() > 128)
			return;

		const std::regex emailPattern(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
		if (!std::regex_match(data.email, emailPattern))
			return;

		// Generate salt
		SecureRandomGenerator gen;

		Nz::ByteArray saltBuff(32, 0);
		if (!gen(saltBuff.GetBuffer(), saltBuff.GetSize()))
		{
			std::cerr << "SecureRandomGenerator failed" << std::endl;

			Packets::RegisterFailure registerFailure;
			registerFailure.reason = RegisterFailureReason::ServerError;

			player->SendPacket(registerFailure);
			return;
		}

		ConfigFile& config = m_app->GetConfig();

		// Salt password and hash it again
		const std::string& globalSalt = config.GetStringOption("Security.PasswordSalt");

		Nz::String userSalt = saltBuff.ToHex();
		Nz::String salt = globalSalt + userSalt;

		int iCost = config.GetIntegerOption<int>("Security.Argon2.IterationCost");
		int mCost = config.GetIntegerOption<int>("Security.Argon2.MemoryCost");
		int tCost = config.GetIntegerOption<int>("Security.Argon2.ThreadCost");
		int hashLength = config.GetIntegerOption<int>("Security.HashLength");

		m_app->DispatchWork([app = m_app, sessionId = player->GetSessionId(), s = std::move(salt), uSalt = std::move(userSalt), data, iCost, mCost, tCost, hashLength]()
		{
			Nz::StackArray<uint8_t> output = NazaraStackArrayNoInit(uint8_t, hashLength);

			argon2_context context;
			std::memset(&context, 0, sizeof(argon2_context));

			context.out = output.data();
			context.outlen = uint32_t(hashLength);
			context.pwd = reinterpret_cast<uint8_t*>(const_cast<char*>(data.passwordHash.data()));
			context.pwdlen = uint32_t(data.passwordHash.size());
			context.salt = reinterpret_cast<uint8_t*>(const_cast<char*>(s.GetConstBuffer()));
			context.saltlen = uint32_t(s.GetSize());
			context.t_cost = iCost;
			context.m_cost = mCost;
			context.lanes = tCost;
			context.threads = tCost;
			context.flags = ARGON2_DEFAULT_FLAGS;
			context.version = ARGON2_VERSION_13;

			std::optional<LoginFailureReason> failure;
			if (argon2_ctx(&context, argon2_type::Argon2_id) == ARGON2_OK)
			{
				std::string outputHex(hashLength * 2 + 1, '\0');

				for (std::size_t i = 0; i < output.size(); ++i)
					std::sprintf(&outputHex[i * 2], "%02x", output[i]);

				outputHex.resize(hashLength * 2);

				app->GetGlobalDatabase().ExecuteStatement("RegisterAccount", { data.login, std::move(outputHex), uSalt.ToStdString(), data.email },
				[app, sessionId, login = data.login](DatabaseResult& result)
				{
					Player* ply = app->GetPlayerBySession(sessionId);
					if (!ply)
						return;

					if (!result.IsValid())
					{
						std::cerr << "RegisterAccount failed: " << result.GetLastErrorMessage() << std::endl;

						Packets::RegisterFailure loginFailure;
						loginFailure.reason = RegisterFailureReason::LoginAlreadyTaken;

						ply->SendPacket(loginFailure);
						return;
					}

					ply->SendPacket(Packets::RegisterSuccess());

					std::cout << "Player #" << ply->GetSession()->GetPeerId() << " registered as " << login << std::endl;
				});
			}
			else
			{
				app->RegisterCallback([app, sessionId]()
				{
					Player* ply = app->GetPlayerBySession(sessionId);
					if (!ply)
						return;

					Packets::RegisterFailure loginFailure;
					loginFailure.reason = RegisterFailureReason::ServerError;

					ply->SendPacket(loginFailure);
				});
			}
		});
	}

	void ClientSession::HandleTimeSyncRequest(const Packets::TimeSyncRequest& data)
	{
		Player* player = GetPlayer();
		if (!player->IsAuthenticated())
			return;

		Packets::TimeSyncResponse response;
		response.requestId = data.requestId;
		response.serverTime = m_app->GetAppTime();

		player->SendPacket(response);
	}

	void ClientSession::HandleUpdateFleet(const Packets::UpdateFleet& data)
	{
		Player* player = GetPlayer();
		if (!player->IsAuthenticated())
			return;

		if (data.fleetName.empty())
			return;

		if (data.spaceships.empty())
			return;

		Nz::Bitset<> usedNames(data.spaceshipNames.size(), false);
		for (const auto& spaceship : data.spaceships)
		{
			if (spaceship.spaceshipNameId >= data.spaceshipNames.size())
				return;

			usedNames[spaceship.spaceshipNameId] = true;

			constexpr float gridSize = 30.f;
			constexpr float maxHeight = 10.f;

			if (spaceship.spaceshipPosition.x < -gridSize  || spaceship.spaceshipPosition.x > gridSize ||
			    spaceship.spaceshipPosition.z < -gridSize  || spaceship.spaceshipPosition.z > gridSize ||
			    spaceship.spaceshipPosition.y < -maxHeight || spaceship.spaceshipPosition.y > maxHeight)
				return;
		}

		// If any name is not used, this may be a forged packet to increase database requests
		if (!usedNames.TestAll())
			return;

		DatabaseTransaction nameTransaction;
		for (const std::string& spaceshipName : data.spaceshipNames)
			nameTransaction.AppendPreparedStatement("FindSpaceshipIdByOwnerIdAndName", { player->GetDatabaseId(), spaceshipName });

		m_app->GetGlobalDatabase().ExecuteTransaction(std::move(nameTransaction), [data, app = m_app, sessionId = GetSessionId()](bool success, std::vector<DatabaseResult>& results)
		{
			Player* ply = app->GetPlayerBySession(sessionId);
			if (!ply)
				return;

			if (!success)
			{
				std::cerr << "Fleet creation id first pass failed: " << results.back().GetLastErrorMessage() << std::endl;

				Packets::UpdateFleetFailure fleetFailure;
				fleetFailure.reason = UpdateFleetFailureReason::ServerError;

				ply->SendPacket(fleetFailure);
				return;
			}

			for (std::size_t i = 1; i < results.size() - 1; ++i)
			{
				if (results[i].GetRowCount() == 0)
				{
					Packets::UpdateFleetFailure fleetFailure;
					fleetFailure.reason = UpdateFleetFailureReason::ServerError;

					ply->SendPacket(fleetFailure);
					return;
				}
			}

			struct SpaceshipData
			{
				Nz::Int32 spaceshipId;
				Nz::Vector3f position;
			};

			std::vector<SpaceshipData> spaceshipData;
			for (std::size_t i = 0; i < data.spaceships.size(); ++i)
			{
				auto& spaceship = spaceshipData.emplace_back();
				spaceship.position = data.spaceships[i].spaceshipPosition;

				std::size_t nameId = data.spaceships[i].spaceshipNameId;
				spaceship.spaceshipId = std::get<Nz::Int32>(results[1 + nameId].GetValue(0));
			}

			DatabaseTransaction fleetTrans;
			fleetTrans.AppendPreparedStatement("FindFleetByOwnerIdAndName", { ply->GetDatabaseId(), data.fleetName }, [data = std::move(spaceshipData), newName = data.newFleetName](DatabaseTransaction& transaction, DatabaseResult result) -> DatabaseResult
			{
				if (!result)
					return result;

				if (result.GetRowCount() == 0)
					return DatabaseResult{};

				// Move data out of lambda memory, because AppendPreparedStatement will move its memory
				auto spaceshipData = std::move(data);

				Nz::Int32 fleetId = std::get<Nz::Int32>(result.GetValue(0));

				if (!newName.empty())
					transaction.AppendPreparedStatement("UpdateFleetNameById", { fleetId, std::move(newName) });

				transaction.AppendPreparedStatement("DeleteFleetSpaceships", { fleetId });
				for (const auto& spaceship : spaceshipData)
					transaction.AppendPreparedStatement("CreateFleetSpaceship", { fleetId, spaceship.spaceshipId, spaceship.position.x, spaceship.position.y, spaceship.position.z });

				transaction.AppendPreparedStatement("UpdateFleetUpdateDate", { fleetId });

				return result;
			});

			app->GetGlobalDatabase().ExecuteTransaction(std::move(fleetTrans), [app, sessionId](bool success, std::vector<DatabaseResult>& results)
			{
				Player* ply = app->GetPlayerBySession(sessionId);
				if (!ply)
					return;

				if (!success)
				{
					Packets::UpdateFleetFailure creationFailed;
					if (results.size() == 2) //< Begin + FindFleetByOwnerIdAndName result
						creationFailed.reason = UpdateFleetFailureReason::NotFound;
					else
						creationFailed.reason = UpdateFleetFailureReason::ServerError;

					ply->SendPacket(creationFailed);
					return;
				}

				ply->SendPacket(Packets::UpdateFleetSuccess());
			});
		});
	}

	void ClientSession::HandleUpdateSpaceship(const Packets::UpdateSpaceship& data)
	{
		Player* player = GetPlayer();
		if (!player->IsAuthenticated())
			return;

		if (data.spaceshipName.empty() || data.spaceshipName.size() > 64)
			return;

		if (data.newSpaceshipName.size() > 64)
			return;

		auto& moduleStore = m_app->GetModuleStore();

		for (const auto& moduleInfo : data.modifiedModules)
		{
			if (moduleStore.GetEntryByName(moduleInfo.moduleName) == ModuleStore::InvalidEntryId)
				return;

			if (moduleStore.GetEntryByName(moduleInfo.oldModuleName) == ModuleStore::InvalidEntryId)
				return;

			if (moduleInfo.type > ModuleType::Max)
				return;
		}

		m_app->GetGlobalDatabase().ExecuteStatement("FindSpaceshipIdByOwnerIdAndName", { player->GetDatabaseId(), data.spaceshipName }, [=, app = m_app, sessionId = player->GetSessionId()](DatabaseResult& result)
		{
			if (!result)
			{
				std::cerr << "FindSpaceshipIdByOwnerIdAndName failed: " << result.GetLastErrorMessage();

				if (Player* ply = app->GetPlayerBySession(sessionId))
				{
					Packets::UpdateSpaceshipFailure response;
					response.reason = UpdateSpaceshipFailureReason::ServerError;

					ply->SendPacket(response);
				}
				return;
			}

			if (result.GetRowCount() == 0)
			{
				if (Player* ply = app->GetPlayerBySession(sessionId))
				{
					Packets::UpdateSpaceshipFailure response;
					response.reason = UpdateSpaceshipFailureReason::NotFound;

					ply->SendPacket(response);
				}

				return;
			}

			Nz::Int32 spaceshipId = std::get<Nz::Int32>(result.GetValue(0));

			DatabaseTransaction transaction;
			if (!data.newSpaceshipName.empty())
				transaction.AppendPreparedStatement("UpdateSpaceshipNameById", { spaceshipId, data.newSpaceshipName });

			if (!data.newSpaceshipCode.empty())
				transaction.AppendPreparedStatement("UpdateSpaceshipScriptById", { spaceshipId, data.newSpaceshipCode });

			if (!data.modifiedModules.empty())
			{
				auto& moduleStore = app->GetModuleStore();

				for (const auto& moduleInfo : data.modifiedModules)
				{
					std::size_t oldModuleId = moduleStore.GetEntryByName(moduleInfo.oldModuleName);
					std::size_t newModuleId = moduleStore.GetEntryByName(moduleInfo.moduleName);

					transaction.AppendPreparedStatement("UpdateSpaceshipModule", { spaceshipId, Nz::Int32(oldModuleId), Nz::Int32(newModuleId) });
				}
			}

			if (transaction.empty())
				return;

			transaction.AppendPreparedStatement("UpdateSpaceshipUpdateDate", { spaceshipId });

			app->GetGlobalDatabase().ExecuteTransaction(std::move(transaction), [app, sessionId](bool transactionSucceeded, std::vector<DatabaseResult>& queryResults)
			{
				Player* ply = app->GetPlayerBySession(sessionId);
				if (!ply)
					return;

				if (!transactionSucceeded)
				{
					std::cerr << "Update spaceship transaction failed: " << queryResults.back().GetLastErrorMessage() << std::endl;

					Packets::UpdateSpaceshipFailure response;
					response.reason = UpdateSpaceshipFailureReason::ServerError;

					ply->SendPacket(response);
					return;
				}

				ply->SendPacket(Packets::UpdateSpaceshipSuccess());
			});
		});
	}

}
