// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerApplication.hpp>
#include <Nazara/Core/File.hpp>
#include <Nazara/Core/MemoryHelper.hpp>
#include <Shared/SecureRandomGenerator.hpp>
#include <Server/Components/OwnerComponent.hpp>
#include <Server/Components/ScriptComponent.hpp>
#include <Server/DatabaseLoader.hpp>
#include <Server/Database/Database.hpp>
#include <Server/Player.hpp>
#include <argon2/argon2.h>
#include <bitset>
#include <cctype>
#include <iostream>
#include <regex>

namespace ewn
{
	ServerApplication::ServerApplication() :
	m_playerPool(sizeof(Player)),
	m_chatCommandStore(this),
	m_commandStore(this),
	m_nextSessionId(0)
	{
		RegisterConfigOptions();
		RegisterNetworkedStrings();

		m_arenas.emplace_back(std::make_unique<Arena>(this, "Le Royaume de Belgique", "arena.lua"));
		m_arenas.emplace_back(std::make_unique<Arena>(this, "La Cinquième République", "arena.lua"));
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

	bool ServerApplication::LoadDatabase()
	{
		Database& globalDatabase = GetGlobalDatabase();

		DatabaseLoader loader;
		loader.RegisterStore("CollisionMeshes", &m_collisionMeshStore, {});
		loader.RegisterStore("Modules", &m_moduleStore, {});
		loader.RegisterStore("SpaceshipHulls", &m_spaceshipHullStore, { "CollisionMeshes", "VisualMeshes" });
		loader.RegisterStore("VisualMeshes", &m_visualMeshStore, {});

		if (!loader.LoadFromDatabase(this, globalDatabase))
			return false;

		// Register mesh paths as networked strings
		for (std::size_t i = 0; i < m_collisionMeshStore.GetEntryCount(); ++i)
		{
			if (m_collisionMeshStore.IsEntryLoaded(i))
				m_stringStore.RegisterString(m_collisionMeshStore.GetEntryFilePath(i));
		}

		for (std::size_t i = 0; i < m_visualMeshStore.GetEntryCount(); ++i)
		{
			if (m_visualMeshStore.IsEntryLoaded(i))
				m_stringStore.RegisterString(m_visualMeshStore.GetEntryFilePath(i));
		}

		if (!BakeDefaultSpaceshipData())
			return false;

		return true;
	}

	bool ServerApplication::Run()
	{
		float updateTime = GetUpdateTime();
		for (const auto& arenaPtr : m_arenas)
			arenaPtr->Update(updateTime);

		m_globalDatabase->Poll();

		ServerCallback func;
		while (m_callbackQueue.try_dequeue(func))
			func();

		return BaseApplication::Run();
	}

	void ServerApplication::HandleControlEntity(std::size_t peerId, const Packets::ControlEntity& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		if (Arena* arena = player->GetArena())
		{
			if (data.id != 0)
			{
				Ndk::EntityId entityId = static_cast<Ndk::EntityId>(data.id);
				if (!arena->IsEntityIdValid(entityId))
				{
					std::cerr << "Client #" << peerId << " tried to control invalid entity #" << entityId << std::endl;
					return;
				}

				const Ndk::EntityHandle& entity = arena->GetEntity(entityId);
				if (!entity->HasComponent<OwnerComponent>() || entity->GetComponent<OwnerComponent>().GetOwner() != player)
				{
					std::cerr << "Client #" << peerId << " tried to control entity #" << entityId << " which doesn't belong to them" << std::endl;
					return;
				}

				player->UpdateControlledEntity(entity);
			}
			else
				player->UpdateControlledEntity(Ndk::EntityHandle::InvalidHandle);
		}
	}

	void ServerApplication::HandleCreateSpaceship(std::size_t peerId, const Packets::CreateSpaceship& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		if (data.spaceshipName.empty())
			return;

		if (data.spaceshipCode.empty())
			return;

		if (data.modules.empty())
			return;

		if (!m_spaceshipHullStore.IsEntryLoaded(data.hullId))
			return;

		std::bitset<static_cast<std::size_t>(ModuleType::Max) + 1> receivedModules;

		std::size_t maxModuleId = m_moduleStore.GetEntryCount() + 1;
		for (const auto& packetModule : data.modules)
		{
			if (packetModule.type > ModuleType::Max)
				return;

			if (packetModule.moduleId > maxModuleId)
				return;

			if (!m_moduleStore.IsEntryLoaded(packetModule.moduleId))
				return;

			if (m_moduleStore.GetEntryType(packetModule.moduleId) != packetModule.type)
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

		std::vector<std::size_t> modules(data.modules.size());
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

	void ServerApplication::HandleDeleteSpaceship(std::size_t peerId, const Packets::DeleteSpaceship & data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		Nz::Int32 playerDatabaseId = Nz::Int32(player->GetDatabaseId());

		DatabaseTransaction trans;
		trans.AppendPreparedStatement("CountSpaceshipByOwnerIdExceptName", { playerDatabaseId, data.spaceshipName }, [this, playerDatabaseId, sessionId = player->GetSessionId(), spaceshipName = data.spaceshipName](DatabaseTransaction& transaction, DatabaseResult result) -> DatabaseResult
		{
			if (!result)
				return result;

			Nz::Int64 spaceshipCount = std::get<Nz::Int64>(result.GetValue(0));
			if (spaceshipCount == 0)
			{
				Player* ply = GetPlayerBySession(sessionId);
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

		m_globalDatabase->ExecuteTransaction(std::move(trans), [this, sessionId = player->GetSessionId()](bool transactionSucceeded, std::vector<DatabaseResult>& queryResults)
		{
			if (!transactionSucceeded)
			{
				// Check if we issued "DeleteSpaceship" statement, if not assume error has already been handled
				if (queryResults.size() < 4)
					return;

				std::cerr << "Delete spaceship transaction failed: " << queryResults.back().GetLastErrorMessage() << std::endl;

				Player* ply = GetPlayerBySession(sessionId);
				if (!ply)
					return;

				Packets::DeleteSpaceshipFailure deleteFailure;
				deleteFailure.reason = DeleteSpaceshipFailureReason::ServerError;

				ply->SendPacket(deleteFailure);
				return;
			}

			Player* ply = GetPlayerBySession(sessionId);
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

	bool ServerApplication::BakeDefaultSpaceshipData()
	{
		// Load informations about default spaceship
		m_defaultSpaceshipData.name = m_config.GetStringOption("DefaultSpaceship.Name");

		// Find hull
		const std::string& hullName = m_config.GetStringOption("DefaultSpaceship.Hull");

		m_defaultSpaceshipData.hullId = m_spaceshipHullStore.GetEntryByName(hullName);
		if (m_defaultSpaceshipData.hullId == m_spaceshipHullStore.InvalidEntryId)
		{
			std::cerr << "Failed to find default spaceship hull \"" << hullName << "\"" << std::endl;
			return false;
		}

		// Find modules
		const std::string& moduleList = m_config.GetStringOption("DefaultSpaceship.Modules");

		auto AddModule = [&](const std::string& moduleName)
		{
			std::size_t moduleId = m_moduleStore.GetEntryByName(moduleName);
			if (moduleId == m_moduleStore.InvalidEntryId)
			{
				std::cerr << "Failed to find default spaceship module \"" << moduleName << "\"" << std::endl;
				return false;
			}

			m_defaultSpaceshipData.moduleIds.push_back(moduleId);
			return true;
		};

		std::size_t pos = 0;
		std::size_t previousPos = 0;
		while ((pos = moduleList.find('|', previousPos)) != std::string::npos)
		{
			if (!AddModule(moduleList.substr(previousPos, pos - previousPos)))
				return false;

			previousPos = pos + 1;
		}

		if (!AddModule(moduleList.substr(previousPos)))
			return false;

		// Load script file
		const std::string& fileName = m_config.GetStringOption("DefaultSpaceship.ScriptFile");

		// Load file content
		Nz::File file(fileName, Nz::OpenMode_ReadOnly | Nz::OpenMode_Text);
		if (!file.IsOpen())
		{
			std::cerr << "Failed to open default spaceship script file \"" << fileName << "\"" << std::endl;
			return false;
		}

		m_defaultSpaceshipData.code.reserve(file.GetSize());

		while (!file.EndOfFile())
		{
			Nz::String fileContent = file.ReadLine();
			m_defaultSpaceshipData.code.append(fileContent.GetConstBuffer(), fileContent.GetSize());
			m_defaultSpaceshipData.code += '\n';
		}

		return true;
	}

	void ServerApplication::HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data)
	{
		const std::unique_ptr<NetworkReactor>& reactor = GetReactor(peerId / GetPeerPerReactor());

		if (peerId >= m_players.size())
			m_players.resize(peerId + 1);

		m_players[peerId] = m_playerPool.New<Player>(this, peerId, m_nextSessionId, *reactor, m_commandStore);
		std::cout << "Client #" << peerId << " connected with data " << data << std::endl;

		m_sessionIdToPlayer.insert_or_assign(m_nextSessionId, peerId);
		m_nextSessionId++;

		// Send newtorked strings
		m_players[peerId]->SendPacket(m_stringStore.BuildPacket(0));
	}

	void ServerApplication::HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data)
	{
		std::cout << "Client #" << peerId << " disconnected with data " << data << std::endl;

		m_sessionIdToPlayer.erase(m_players[peerId]->GetSessionId());

		m_playerPool.Delete(m_players[peerId]);
		m_players[peerId] = nullptr;
	}

	void ServerApplication::HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet)
	{
		//std::cout << "Client #" << peerId << " sent packet of size " << packet.GetDataSize() << std::endl;

		if (!m_commandStore.UnserializePacket(peerId, std::move(packet)))
			m_players[peerId]->Disconnect();
	}

	void ServerApplication::HandleLoginSucceeded(Player* player, Nz::Int32 databaseId, bool regenerateToken)
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

		RegisterCallback([this, sessionId = player->GetSessionId(), databaseId, connectionToken = std::move(token)]()
		{
			Player* ply = GetPlayerBySession(sessionId);
			if (!ply)
				return;

			ply->Authenticate(databaseId, [this, playerToken = std::move(connectionToken)](Player* player, bool loginSuccess)
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

						m_globalDatabase->ExecuteTransaction(std::move(dbTransaction), [this, packetToken = std::move(playerToken), sessionId = player->GetSessionId()](bool transactionSucceeded, std::vector<DatabaseResult>& queryResults)
						{
							Player* player = GetPlayerBySession(sessionId);
							if (!player)
								return;

							if (transactionSucceeded)
							{
								Packets::LoginSuccess loginSuccess;
								loginSuccess.connectionToken = std::move(packetToken);

								player->SendPacket(loginSuccess);
								std::cout << "Player #" << player->GetPeerId() << " authenticated as " << player->GetName() << " and regenerated a connection token" << std::endl;
							}
							else
							{
								std::cout << "Failed to save token: " << queryResults.back().GetLastErrorMessage() << std::endl;
								player->SendPacket(Packets::LoginSuccess());
								std::cout << "Player #" << player->GetPeerId() << " authenticated as " << player->GetName() << std::endl;
							}
						});
					}
					else
					{
						player->SendPacket(Packets::LoginSuccess());
						std::cout << "Player #" << player->GetPeerId() << " authenticated as " << player->GetName() << std::endl;
					}
				}
				else
				{
					std::cerr << "Failed to authenticate player #" << player->GetPeerId() << ": Database authentication failed" << std::endl;

					Packets::LoginFailure loginFailure;
					loginFailure.reason = LoginFailureReason::ServerError;

					player->SendPacket(loginFailure);
				}
			});
		});
	}

	void ServerApplication::InitGameWorkers(std::size_t workerCount)
	{
		m_workers.reserve(workerCount);
		for (std::size_t i = 0; i < workerCount; ++i)
			m_workers.emplace_back(std::make_unique<GameWorker>(this));
	}

	void ServerApplication::InitGlobalDatabase(std::size_t workerCount, std::string dbHost, Nz::UInt16 port, std::string dbUser, std::string dbPassword, std::string dbName)
	{
		m_globalDatabase.emplace(std::move(dbHost), port, std::move(dbUser), std::move(dbPassword), std::move(dbName));
		m_globalDatabase->SpawnWorkers(workerCount);
	}

	void ServerApplication::OnConfigLoaded(const ConfigFile& config)
	{
		const std::string& dbHost = m_config.GetStringOption("Database.Host");
		const std::string& dbUser = m_config.GetStringOption("Database.Username");
		const std::string& dbPassword = m_config.GetStringOption("Database.Password");
		const std::string& dbName = m_config.GetStringOption("Database.Name");
		Nz::UInt16 dbPort = m_config.GetIntegerOption<Nz::UInt16>("Database.Port");
		std::size_t dbWorkerCount = m_config.GetIntegerOption<std::size_t>("Database.WorkerCount");

		std::size_t gameWorkerCount = m_config.GetIntegerOption<std::size_t>("Game.WorkerCount");

		InitGameWorkers(gameWorkerCount);
		InitGlobalDatabase(dbWorkerCount, dbHost, dbPort, dbUser, dbPassword, dbName);
	}

	void ServerApplication::HandleLogin(std::size_t peerId, const Packets::Login& data)
	{
		Player* player = m_players[peerId];
		if (player->IsAuthenticated())
			return;

		if (data.login.empty() || data.login.size() > 20)
			return;

		m_globalDatabase->ExecuteQuery("FindAccountByLogin", { data.login },
		[this, sessionId = player->GetSessionId(), login = data.login, pwd = data.passwordHash, needToken = data.generateConnectionToken](DatabaseResult& result)
		{
			Player* ply = GetPlayerBySession(sessionId);
			if (!ply)
				return;

			if (!result.IsValid())
			{
				std::cerr << "FindAccountByLogin failed: " << result.GetLastErrorMessage() << std::endl;

				Packets::LoginFailure loginFailure;
				loginFailure.reason = LoginFailureReason::ServerError;

				ply->SendPacket(loginFailure);
				return;
			}

			if (result.GetRowCount() == 0)
			{
				std::cout << "Player #" << ply->GetPeerId() << " authentication as " << login << " failed: player not found" << std::endl;

				Packets::LoginFailure loginFailure;
				loginFailure.reason = LoginFailureReason::AccountNotFound;

				ply->SendPacket(loginFailure);
				return;
			}

			assert(result.GetRowCount() == 1);

			const std::string& globalSalt = m_config.GetStringOption("Security.PasswordSalt");

			Nz::Int32 dbId = std::get<Nz::Int32>(result.GetValue(0));
			std::string dbPassword = std::get<std::string>(result.GetValue(1));
			std::string dbSalt = std::get<std::string>(result.GetValue(2));
			std::string salt = globalSalt + dbSalt;

			int iCost = m_config.GetIntegerOption<int>("Security.Argon2.IterationCost");
			int mCost = m_config.GetIntegerOption<int>("Security.Argon2.MemoryCost");
			int tCost = m_config.GetIntegerOption<int>("Security.Argon2.ThreadCost");
			int hashLength = m_config.GetIntegerOption<int>("Security.HashLength");

			DispatchWork([this, s = std::move(salt), pass = std::move(pwd), dbPass = dbPassword, id = dbId, sessionId, login, iCost, mCost, tCost, hashLength, needToken]()
			{
				Nz::StackArray<uint8_t> output = NazaraStackAllocationNoInit(uint8_t, hashLength);
				Nz::StackArray<char> outputHex = NazaraStackAllocationNoInit(char, hashLength * 2 + 1);

				argon2_context context;
				std::memset(&context, 0, sizeof(argon2_context));

				context.out = output.data();
				context.outlen = uint32_t(hashLength);
				context.pwd = reinterpret_cast<uint8_t*>(const_cast<char*>(pass.data()));
				context.pwdlen = uint32_t(pass.size());
				context.salt = reinterpret_cast<uint8_t*>(const_cast<char*>(s.data()));
				context.saltlen = uint32_t(s.size());
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
					Player* ply = GetPlayerBySession(sessionId);
					if (!ply)
						return;

					HandleLoginSucceeded(ply, id, needToken);
				}
				else
				{
					RegisterCallback([this, sessionId, login, reason = failure.value(), argon2Ret]()
					{
						Player* ply = GetPlayerBySession(sessionId);
						if (!ply)
							return;

						Packets::LoginFailure loginFailure;
						loginFailure.reason = reason;

						ply->SendPacket(loginFailure);

						switch (reason)
						{
							case LoginFailureReason::PasswordMismatch:
								std::cout << "Player #" << ply->GetPeerId() << " authentication as " << login << " failed: password mismatch" << std::endl;
								break;

							case LoginFailureReason::ServerError:
								std::cout << "Player #" << ply->GetPeerId() << " authentication as " << login << " failed: argon2 failure (err: " << argon2Ret << ")" << std::endl;
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

	void ServerApplication::HandleLoginByToken(std::size_t peerId, const Packets::LoginByToken& data)
	{
		Player* player = m_players[peerId];
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
		m_globalDatabase->ExecuteTransaction(std::move(trans), [this, sessionId = player->GetSessionId(), accountResultId, generateNewToken = data.generateConnectionToken](bool transactionSucceeded, std::vector<DatabaseResult>& queryResults)
		{
			Player* ply = GetPlayerBySession(sessionId);
			if (!ply)
				return;

			if (!transactionSucceeded || queryResults[accountResultId].GetRowCount() == 0)
			{
				std::cout << "Player #" << ply->GetPeerId() << " authentication via token failed" << std::endl;

				Packets::LoginFailure loginFailure;
				loginFailure.reason = LoginFailureReason::InvalidToken;

				ply->SendPacket(loginFailure);
				return;
			}

			Nz::Int32 dbId = std::get<Nz::Int32>(queryResults[accountResultId].GetValue(0));
			HandleLoginSucceeded(ply, dbId, generateNewToken);
		});
	}

	void ServerApplication::HandleLeaveArena(std::size_t peerId, const Packets::LeaveArena& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		if (player->GetArena())
			player->MoveToArena(nullptr);
	}

	void ServerApplication::HandleJoinArena(std::size_t peerId, const Packets::JoinArena& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		if (data.arenaIndex > m_arenas.size())
			return;

		Arena* arena = m_arenas[data.arenaIndex].get();
		if (player->GetArena() != arena)
			player->MoveToArena(arena);
	}

	void ServerApplication::HandlePlayerChat(std::size_t peerId, const Packets::PlayerChat& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		if (data.text.empty())
			return;

		if (data.text[0] == '/')
		{
			std::string_view command = data.text;
			command.remove_prefix(1);

			std::optional<bool> result = m_chatCommandStore.ExecuteCommand(player, command);
			if (result)
			{
				if (!*result)
					player->PrintMessage("Error in parsing command, you may be missing parameters");

				return; // Don't show command if it was recognized
			}
		}

		if (Arena* arena = player->GetArena())
		{
			static constexpr std::size_t MaxChatLine = 255;

			Nz::String message = player->GetName() + ": " + data.text;
			if (message.GetSize() > MaxChatLine)
			{
				message.Resize(MaxChatLine - 3, Nz::String::HandleUtf8);
				message += "...";
			}

			std::cout << message << std::endl;

			arena->DispatchChatMessage(message);
		}
	}

	void ServerApplication::HandlePlayerMovement(std::size_t peerId, const Packets::PlayerMovement& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		player->UpdateInput(data.inputTime, data.direction, data.rotation);
	}

	void ServerApplication::HandlePlayerShoot(std::size_t peerId, const Packets::PlayerShoot& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		player->Shoot();
	}

	void ServerApplication::HandleQueryArenaList(std::size_t peerId, const Packets::QueryArenaList& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		Packets::ArenaList listPacket;
		listPacket.arenas.reserve(m_arenas.size());

		for (const auto& arenaPtr : m_arenas)
		{
			auto& arenaData = listPacket.arenas.emplace_back();
			arenaData.arenaName = arenaPtr->GetName();
		}

		player->SendPacket(listPacket);
	}

	void ServerApplication::HandleQueryHullList(std::size_t peerId, const Packets::QueryHullList& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		Packets::HullList hullList;
		hullList.hulls.reserve(m_spaceshipHullStore.GetEntryCount());

		for (std::size_t i = 0; i < m_spaceshipHullStore.GetEntryCount(); ++i)
		{
			if (m_spaceshipHullStore.IsEntryLoaded(i))
			{
				std::size_t visualMeshId = m_spaceshipHullStore.GetEntryVisualMeshId(i);

				auto& hullInfo = hullList.hulls.emplace_back();
				hullInfo.description = m_spaceshipHullStore.GetEntryDescription(i);
				hullInfo.hullId = static_cast<Nz::UInt32>(i);
				hullInfo.hullModelPathId = m_stringStore.GetStringIndex(m_visualMeshStore.GetEntryFilePath(i));
				hullInfo.name = m_spaceshipHullStore.GetEntryName(i);

				hullInfo.slots.reserve(m_spaceshipHullStore.GetEntrySlotCount(i));

				for (std::size_t j = 0; j < m_spaceshipHullStore.GetEntrySlotCount(i); ++j)
				{
					auto& slotInfo = hullInfo.slots.emplace_back();
					slotInfo.type = m_spaceshipHullStore.GetEntrySlotModuleType(i, j);
				}
			}
		}

		player->SendPacket(hullList);
	}

	void ServerApplication::HandleQueryModuleList(std::size_t peerId, const Packets::QueryModuleList& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		Packets::ModuleList moduleList;

		for (std::size_t i = 0; i < m_moduleStore.GetEntryCount(); ++i)
		{
			if (m_moduleStore.IsEntryLoaded(i))
			{
				ModuleType type = m_moduleStore.GetEntryType(i);

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
				moduleInfo.moduleName = m_moduleStore.GetEntryName(i);
			}
		}

		player->SendPacket(moduleList);
	}

	void ServerApplication::HandleQuerySpaceshipInfo(std::size_t peerId, const Packets::QuerySpaceshipInfo& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		if (data.spaceshipName.empty())
			return;

		m_globalDatabase->ExecuteQuery("FindSpaceshipByOwnerIdAndName", { player->GetDatabaseId(), data.spaceshipName }, [&, sessionId = player->GetSessionId()](DatabaseResult& result)
		{
			Player* ply = GetPlayerBySession(sessionId);
			if (!ply)
				return; //< Player has disconnected, ignore

			if (!result)
			{
				std::cerr << "FindSpaceshipByOwnerIdAndName failed: " << result.GetLastErrorMessage() << std::endl;

				ply->SendPacket(Packets::SpaceshipInfo{});
				return;
			}

			if (result.GetRowCount() == 0)
			{
				ply->SendPacket(Packets::SpaceshipInfo{});
				return;
			}

			Nz::Int32 spaceshipId = std::get<Nz::Int32>(result.GetValue(0));

			Nz::UInt32 spaceshipHullId = static_cast<Nz::UInt32>(std::get<Nz::Int32>(result.GetValue(2)));
			std::size_t visualMeshId = m_spaceshipHullStore.GetEntryVisualMeshId(spaceshipHullId);

			m_globalDatabase->ExecuteQuery("FindSpaceshipModulesBySpaceshipId", { spaceshipId }, [=](DatabaseResult& result)
			{
				if (!ply)
					return; //< Player has disconnected, ignore

				Packets::SpaceshipInfo spaceshipInfo;

				if (result.IsValid())
				{
					spaceshipInfo.hullId = spaceshipHullId;
					spaceshipInfo.hullModelPath = m_visualMeshStore.GetEntryFilePath(visualMeshId);

					if (!result)
					{
						ply->PrintMessage("Server: Failed to retrieve spaceship modules, please contact an administrator");
						return;
					}

					// Spaceship actual modules
					try
					{
						std::size_t moduleCount = result.GetRowCount();

						spaceshipInfo.modules.reserve(moduleCount);
						for (std::size_t i = 0; i < moduleCount; ++i)
						{
							Nz::UInt32 moduleId = static_cast<Nz::UInt32>(std::get<Nz::Int32>(result.GetValue(0, i)));

							auto& moduleInfo = spaceshipInfo.modules.emplace_back();
							moduleInfo.type = m_moduleStore.GetEntryType(moduleId);
							moduleInfo.currentModule = moduleId;
						}
					}
					catch (const std::exception& e)
					{
						std::cerr << "Failed to retrieve spaceship modules: " << e.what() << std::endl;
						ply->SendPacket(Packets::SpaceshipInfo{});
						return;
					}
				}
				else
					std::cerr << "FindSpaceshipByOwnerIdAndName failed:" << result.GetLastErrorMessage() << std::endl;

				ply->SendPacket(spaceshipInfo);
			});
		});
	}

	void ServerApplication::HandleQuerySpaceshipList(std::size_t peerId, const Packets::QuerySpaceshipList& /*data*/)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		m_globalDatabase->ExecuteQuery("FindSpaceshipsByOwnerId", { Nz::Int32(player->GetDatabaseId()) }, [this, sessionId = player->GetSessionId()](DatabaseResult& result)
		{
			Player* ply = GetPlayerBySession(sessionId);
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
					const auto& defaultSpaceshipData = GetDefaultSpaceshipData();

					ply->CreateSpaceship(defaultSpaceshipData.name, defaultSpaceshipData.code, defaultSpaceshipData.hullId, defaultSpaceshipData.moduleIds, [this](Player* player, bool succeeded)
					{
						if (!player)
							return;

						Packets::SpaceshipList spaceshipList;
						if (succeeded)
						{
							// Fill list manually
							spaceshipList.spaceships.resize(1);
							spaceshipList.spaceships.back().name = GetDefaultSpaceshipData().name;
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

	void ServerApplication::HandleRegister(std::size_t peerId, const Packets::Register& data)
	{
		Player* player = m_players[peerId];
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

		// Salt password and hash it again
		const std::string& globalSalt = m_config.GetStringOption("Security.PasswordSalt");

		Nz::String userSalt = saltBuff.ToHex();
		Nz::String salt = globalSalt + userSalt;

		int iCost = m_config.GetIntegerOption<int>("Security.Argon2.IterationCost");
		int mCost = m_config.GetIntegerOption<int>("Security.Argon2.MemoryCost");
		int tCost = m_config.GetIntegerOption<int>("Security.Argon2.ThreadCost");
		int hashLength = m_config.GetIntegerOption<int>("Security.HashLength");

		DispatchWork([this, sessionId = player->GetSessionId(), s = std::move(salt), uSalt = std::move(userSalt), data, iCost, mCost, tCost, hashLength]()
		{
			Nz::StackArray<uint8_t> output = NazaraStackAllocationNoInit(uint8_t, hashLength);

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

				m_globalDatabase->ExecuteQuery("RegisterAccount", { data.login, std::move(outputHex), uSalt.ToStdString(), data.email },
				[this, sessionId, login = data.login](DatabaseResult& result)
				{
					Player* ply = GetPlayerBySession(sessionId);
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

					std::cout << "Player #" << ply->GetPeerId() << " registered as " << login << std::endl;
				});
			}
			else
			{
				RegisterCallback([this, sessionId]()
				{
					Player* ply = GetPlayerBySession(sessionId);
					if (!ply)
						return;

					Packets::RegisterFailure loginFailure;
					loginFailure.reason = RegisterFailureReason::ServerError;

					ply->SendPacket(loginFailure);
				});
			}
		});
	}

	void ServerApplication::HandleTimeSyncRequest(std::size_t peerId, const Packets::TimeSyncRequest& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		Packets::TimeSyncResponse response;
		response.requestId = data.requestId;
		response.serverTime = GetAppTime();

		player->SendPacket(response);
	}

	void ServerApplication::HandleUpdateSpaceship(std::size_t peerId, const Packets::UpdateSpaceship& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		if (data.spaceshipName.empty() || data.spaceshipName.size() > 64)
			return;

		if (data.newSpaceshipName.size() > 64)
			return;

		for (const auto& moduleInfo : data.modifiedModules)
		{
			if (m_moduleStore.GetEntryByName(moduleInfo.moduleName) == ModuleStore::InvalidEntryId)
				return;

			if (m_moduleStore.GetEntryByName(moduleInfo.oldModuleName) == ModuleStore::InvalidEntryId)
				return;

			if (moduleInfo.type > ModuleType::Max)
				return;
		}

		m_globalDatabase->ExecuteQuery("FindSpaceshipIdByOwnerIdAndName", { player->GetDatabaseId(), data.spaceshipName }, [=, sessionId = player->GetSessionId()](DatabaseResult& result)
		{
			if (!result)
			{
				std::cerr << "FindSpaceshipIdByOwnerIdAndName failed: " << result.GetLastErrorMessage();

				if (Player* ply = GetPlayerBySession(sessionId))
				{
					Packets::UpdateSpaceshipFailure response;
					response.reason = UpdateSpaceshipFailureReason::ServerError;

					ply->SendPacket(response);
				}
				return;
			}

			if (result.GetRowCount() == 0)
			{
				if (Player* ply = GetPlayerBySession(sessionId))
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
				for (const auto& moduleInfo : data.modifiedModules)
				{
					std::size_t oldModuleId = m_moduleStore.GetEntryByName(moduleInfo.oldModuleName);
					std::size_t newModuleId = m_moduleStore.GetEntryByName(moduleInfo.moduleName);

					transaction.AppendPreparedStatement("UpdateSpaceshipModule", { spaceshipId, Nz::Int32(oldModuleId), Nz::Int32(newModuleId) });
				}
			}

			if (transaction.empty())
				return;

			transaction.AppendPreparedStatement("UpdateSpaceshipUpdateDate", { spaceshipId });

			m_globalDatabase->ExecuteTransaction(std::move(transaction), [this, sessionId](bool transactionSucceeded, std::vector<DatabaseResult>& queryResults)
			{
				Player* ply = GetPlayerBySession(sessionId);
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

	bool ServerApplication::SetupNetwork(std::size_t clientPerReactor, std::size_t reactorCount, Nz::NetProtocol protocol, Nz::UInt16 firstPort)
	{
		m_peerPerReactor = clientPerReactor;

		ClearReactors();
		try
		{
			for (std::size_t i = 0; i < reactorCount; ++i)
				AddReactor(std::make_unique<NetworkReactor>(m_peerPerReactor * i, protocol, Nz::UInt16(firstPort + i), clientPerReactor));

			return true;
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to start network reactors: " << e.what() << std::endl;
			return false;
		}
	}

	void ServerApplication::RegisterConfigOptions()
	{
		m_config.RegisterStringOption("AssetsFolder");

		// Database configuration
		m_config.RegisterStringOption("Database.Host");
		m_config.RegisterStringOption("Database.Name");
		m_config.RegisterStringOption("Database.Password");
		m_config.RegisterIntegerOption("Database.Port", 1, 0xFFFF);
		m_config.RegisterStringOption("Database.Username");
		m_config.RegisterIntegerOption("Database.WorkerCount", 1, 100);

		m_config.RegisterIntegerOption("Security.Argon2.IterationCost");
		m_config.RegisterIntegerOption("Security.Argon2.MemoryCost");
		m_config.RegisterIntegerOption("Security.Argon2.ThreadCost");
		m_config.RegisterIntegerOption("Security.HashLength");
		m_config.RegisterStringOption("Security.PasswordSalt");

		m_config.RegisterIntegerOption("Game.MaxClients", 0, 4096); //< 4096 due to ENet limitation
		m_config.RegisterIntegerOption("Game.Port", 1, 0xFFFF);
		m_config.RegisterIntegerOption("Game.WorkerCount", 1, 100);

		m_config.RegisterStringOption("DefaultSpaceship.Hull");
		m_config.RegisterStringOption("DefaultSpaceship.Modules");
		m_config.RegisterStringOption("DefaultSpaceship.Name");
		m_config.RegisterStringOption("DefaultSpaceship.ScriptFile");
	}

	void ServerApplication::RegisterNetworkedStrings()
	{
		m_stringStore.RegisterString("earth");
		m_stringStore.RegisterString("light");
		m_stringStore.RegisterString("plasmabeam");
		m_stringStore.RegisterString("torpedo");
		m_stringStore.RegisterString("explosion_flare");
		m_stringStore.RegisterString("explosion_fire");
		m_stringStore.RegisterString("explosion_smoke");
		m_stringStore.RegisterString("explosion_wave");
	}
}
