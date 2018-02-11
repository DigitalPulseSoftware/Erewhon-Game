// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerApplication.hpp>
#include <Shared/SecureRandomGenerator.hpp>
#include <Server/Components/ScriptComponent.hpp>
#include <Server/Database/Database.hpp>
#include <Server/Player.hpp>
#include <cctype>
#include <iostream>

namespace ewn
{
	ServerApplication::ServerApplication() :
	m_playerPool(sizeof(Player)),
	m_arena(this),
	m_chatCommandStore(this),
	m_commandStore(this)
	{
		// Database configuration
		m_config.RegisterStringOption("Database.Host");
		m_config.RegisterStringOption("Database.Name");
		m_config.RegisterStringOption("Database.Password");
		m_config.RegisterIntegerOption("Database.Port", 1, 0xFFFF);
		m_config.RegisterStringOption("Database.Username");
		m_config.RegisterIntegerOption("Database.WorkerCount", 1, 100);

		m_config.RegisterIntegerOption("Game.MaxClients", 0, 4096); //< 4096 due to ENet limitation
		m_config.RegisterIntegerOption("Game.Port", 1, 0xFFFF);
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

	Database& ServerApplication::GetGlobalDatabase()
	{
		assert(m_globalDatabase.has_value());
		return *m_globalDatabase;
	}

	bool ServerApplication::Run()
	{
		m_arena.Update(GetUpdateTime());

		m_globalDatabase->Poll();

		return BaseApplication::Run();
	}

	void ServerApplication::HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data)
	{
		const std::unique_ptr<NetworkReactor>& reactor = GetReactor(peerId / GetPeerPerReactor());

		if (peerId >= m_players.size())
			m_players.resize(peerId + 1);

		m_players[peerId] = m_playerPool.New<Player>(this, peerId, *reactor, m_commandStore);
		std::cout << "Client #" << peerId << " connected with data " << data << std::endl;
	}

	void ServerApplication::HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data)
	{
		std::cout << "Client #" << peerId << " disconnected with data " << data << std::endl;

		m_playerPool.Delete(m_players[peerId]);
		m_players[peerId] = nullptr;
	}

	void ServerApplication::HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet)
	{
		//std::cout << "Client #" << peerId << " sent packet of size " << packet.GetDataSize() << std::endl;

		if (!m_commandStore.UnserializePacket(peerId, std::move(packet)))
			m_players[peerId]->Disconnect();
	}

	void ServerApplication::OnConfigLoaded(const ConfigFile& config)
	{
		const std::string& dbHost = m_config.GetStringOption("Database.Host");
		const std::string& dbUser = m_config.GetStringOption("Database.Username");
		const std::string& dbPassword = m_config.GetStringOption("Database.Password");
		const std::string& dbName = m_config.GetStringOption("Database.Name");
		long long dbPort = m_config.GetIntegerOption("Database.Port");
		long long workerCount = m_config.GetIntegerOption("Database.WorkerCount");

		InitGlobalDatabase(workerCount, dbHost, dbPort, dbUser, dbPassword, dbName);
	}

	void ServerApplication::InitGlobalDatabase(std::size_t workerCount, std::string dbHost, Nz::UInt16 port, std::string dbUser, std::string dbPassword, std::string dbName)
	{
		m_globalDatabase.emplace(std::move(dbHost), port, std::move(dbUser), std::move(dbPassword), std::move(dbName));
		m_globalDatabase->SpawnWorkers(workerCount);
	}

	void ServerApplication::HandleLogin(std::size_t peerId, const Packets::Login& data)
	{
		Player* player = m_players[peerId];
		if (player->IsAuthenticated())
			return;

		if (data.login.empty() || data.login.size() > 20)
			return;

		m_globalDatabase->ExecuteQuery("FindAccountByLogin", { data.login },
		[ply = player->CreateHandle(), login = data.login, pwd = data.passwordHash, this](DatabaseResult& result)
		{
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

			std::string dbPassword = std::get<std::string>(result.GetValue(0, 0));
			std::string dbSalt = std::get<std::string>(result.GetValue(1, 0));

			// Salt password and hash it again
			Nz::String saltedPassword = Nz::ComputeHash(Nz::HashType_SHA256, pwd + dbSalt).ToHex();

			if (saltedPassword == std::get<std::string>(result.GetValue(0, 0)))
			{
				ply->Authenticate(login);

				ply->SendPacket(Packets::LoginSuccess());

				std::cout << "Player #" << ply->GetPeerId() << " authenticated as " << login << std::endl;
			}
			else
			{
				Packets::LoginFailure loginFailure;
				loginFailure.reason = LoginFailureReason::PasswordMismatch;

				ply->SendPacket(loginFailure);

				std::cout << "Player #" << ply->GetPeerId() << " authentication as " << login << " failed: password mismatch" << std::endl;
			}
		});
	}

	void ServerApplication::HandleJoinArena(std::size_t peerId, const Packets::JoinArena& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		Arena* arena = &m_arena; //< One arena atm
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

			if (m_chatCommandStore.ExecuteCommand(command, player))
				return; // Don't show command if it succeeded
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

		// Generate salt
		SecureRandomGenerator gen;

		Nz::ByteArray saltBuff(16, 0);
		if (!gen(saltBuff.GetBuffer(), saltBuff.GetSize()))
		{
			std::cerr << "SecureRandomGenerator failed" << std::endl;

			Packets::RegisterFailure registerFailure;
			registerFailure.reason = RegisterFailureReason::ServerError;

			player->SendPacket(registerFailure);
			return;
		}

		// Salt password and hash it again
		Nz::String salt = saltBuff.ToHex();
		Nz::String saltedPassword = Nz::ComputeHash(Nz::HashType_SHA256, data.passwordHash + salt).ToHex();

		m_globalDatabase->ExecuteQuery("RegisterAccount", { data.login, saltedPassword.ToStdString(), salt.ToStdString(), data.email },
		[ply = player->CreateHandle(), login = data.login](DatabaseResult& result)
		{
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

	void ServerApplication::HandleUploadScript(std::size_t peerId, const Packets::UploadScript& data)
	{
		Player* player = m_players[peerId];
		if (!player->IsAuthenticated())
			return;

		const Ndk::EntityHandle& playerBot = player->GetBotEntity();
		ScriptComponent& botScript = playerBot->AddComponent<ScriptComponent>();

		Nz::String lastError;
		if (botScript.Execute(data.code, &lastError))
		{
			Packets::ChatMessage chatPacket;
			chatPacket.message = "Server: Script uploaded with success";

			player->SendPacket(chatPacket);
		}
		else
		{
			Packets::ChatMessage chatPacket;
			chatPacket.message = "Server: Failed to execute script: " + lastError.ToStdString();

			player->SendPacket(chatPacket);
		}
	}
}
