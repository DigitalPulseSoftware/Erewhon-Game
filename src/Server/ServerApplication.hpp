// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_APPLICATION_HPP
#define EREWHON_SERVER_APPLICATION_HPP

#include <Shared/BaseApplication.hpp>
#include <Shared/Protocol/NetworkStringStore.hpp>
#include <Nazara/Core/MemoryPool.hpp>
#include <Server/Arena.hpp>
#include <Server/GameWorker.hpp>
#include <Server/GlobalDatabase.hpp>
#include <Server/ServerCommandStore.hpp>
#include <Server/ServerChatCommandStore.hpp>
#include <Server/Store/CollisionMeshStore.hpp>
#include <Server/Store/ModuleStore.hpp>
#include <Server/Store/SpaceshipHullStore.hpp>
#include <Server/Store/VisualMeshStore.hpp>
#include <optional>
#include <vector>

namespace ewn
{
	class ServerApplication final : public BaseApplication
	{
		friend GameWorker;

		public:
			struct DefaultSpaceship;
			using ServerCallback = std::function<void()>;
			using WorkerFunction = std::function<void()>;

			ServerApplication();
			virtual ~ServerApplication();

			Arena& CreateArena(std::string name, std::string script);

			inline void DispatchWork(WorkerFunction workFunc);

			inline Arena* GetArena(std::size_t arenaIndex) const;
			inline std::size_t GetArenaCount() const;
			inline ServerChatCommandStore& GetChatCommandStore();
			inline const ServerChatCommandStore& GetChatCommandStore() const;
			inline CollisionMeshStore& GetCollisionMeshStore();
			inline const CollisionMeshStore& GetCollisionMeshStore() const;
			inline const DefaultSpaceship& GetDefaultSpaceshipData() const;
			inline Database& GetGlobalDatabase();
			inline ModuleStore& GetModuleStore();
			inline const ModuleStore& GetModuleStore() const;
			inline std::size_t GetPeerPerReactor() const;
			inline Player* GetPlayerBySession(std::size_t sessionId);
			inline const NetworkStringStore& GetNetworkStringStore() const;
			inline SpaceshipHullStore& GetSpaceshipHullStore();
			inline const SpaceshipHullStore& GetSpaceshipHullStore() const;
			inline VisualMeshStore& GetVisualMeshStore();
			inline const VisualMeshStore& GetVisualMeshStore() const;

			bool LoadDatabase();

			bool Run() override;

			inline void RegisterCallback(ServerCallback callback);

			bool SetupNetwork(std::size_t clientPerReactor, std::size_t reactorCount, Nz::NetProtocol protocol, Nz::UInt16 firstPort);

			struct DefaultSpaceship
			{
				std::string name;
				std::string code;
				std::size_t hullId;
				std::vector<std::size_t> moduleIds;
			};

		private:
			using CallbackQueue = moodycamel::ConcurrentQueue<ServerCallback>;
			using WorkerQueue = moodycamel::BlockingConcurrentQueue<WorkerFunction>;

			bool BakeDefaultSpaceshipData();

			inline WorkerQueue& GetWorkerQueue();

			void HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data) override;
			void HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data) override;
			void HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet) override;

			void InitGameWorkers(std::size_t workerCount);
			void InitGlobalDatabase(std::size_t workerCount, std::string dbHost, Nz::UInt16 port, std::string dbUser, std::string dbPassword, std::string dbName);

			void OnConfigLoaded(const ConfigFile& config) override;

			void RegisterConfigOptions();
			void RegisterNetworkedStrings();

			std::optional<GlobalDatabase> m_globalDatabase;
			std::size_t m_peerPerReactor;
			std::size_t m_nextSessionId;
			std::unordered_map<std::size_t /*sessionId*/, std::size_t /*peerId*/> m_sessionIdToPeer;
			std::vector<std::unique_ptr<GameWorker>> m_workers;
			std::vector<ClientSession*> m_sessions;
			std::vector<std::unique_ptr<Arena>> m_arenas;
			Nz::MemoryPool m_sessionPool;
			CallbackQueue m_callbackQueue;
			CollisionMeshStore m_collisionMeshStore;
			DefaultSpaceship m_defaultSpaceshipData;
			ModuleStore m_moduleStore;
			NetworkStringStore m_stringStore;
			ServerChatCommandStore m_chatCommandStore;
			ServerCommandStore m_commandStore;
			SpaceshipHullStore m_spaceshipHullStore;
			VisualMeshStore m_visualMeshStore;
			WorkerQueue m_workerQueue;
	};
}

#include <Server/ServerApplication.inl>

#endif // EREWHON_SERVER_APPLICATION_HPP
