// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerApplication.hpp>
#include <Nazara/Core/File.hpp>
#include <Server/DatabaseLoader.hpp>
#include <Server/Player.hpp>
#include <iostream>

namespace ewn
{
	ServerApplication::ServerApplication() :
	m_sessionPool(sizeof(ClientSession)),
	m_chatCommandStore(this),
	m_nextSessionId(0)
	{
		RegisterConfigOptions();
		RegisterNetworkedStrings();
	}

	ServerApplication::~ServerApplication()
	{
		for (ClientSession* session : m_sessions)
		{
			if (session)
			{
				session->Disconnect();
				m_sessionPool.Delete(session);
			}
		}
	}

	Arena& ServerApplication::CreateArena(std::string name, std::string script)
	{
		m_arenas.emplace_back(std::make_unique<Arena>(this, std::move(name), std::move(script)));
		return *m_arenas.back().get();
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

		if (peerId >= m_sessions.size())
			m_sessions.resize(peerId + 1);

		auto player = std::make_shared<Player>(this);

		std::size_t sessionId = m_nextSessionId;
		m_sessionIdToPeer.insert_or_assign(sessionId, peerId);
		m_nextSessionId++;

		m_sessions[peerId] = m_sessionPool.New<ClientSession>(this, sessionId, peerId, player, *reactor, m_commandStore);

		player->UpdateSession(m_sessions[peerId]);

		std::cout << "Client #" << peerId << " (sess. " << sessionId << ") connected with data " << data << std::endl;

		// Send networked strings
		m_sessions[peerId]->SendPacket(m_stringStore.BuildPacket(0));
	}

	void ServerApplication::HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data)
	{
		std::cout << "Client #" << peerId << " disconnected with data " << data << std::endl;

		m_sessionIdToPeer.erase(m_sessions[peerId]->GetSessionId());

		m_sessionPool.Delete(m_sessions[peerId]);
		m_sessions[peerId] = nullptr;
	}

	void ServerApplication::HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet)
	{
		//std::cout << "Client #" << peerId << " sent packet of size " << packet.GetDataSize() << std::endl;

		if (!m_commandStore.UnserializePacket(*m_sessions[peerId], std::move(packet)))
			m_sessions[peerId]->Disconnect();
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
