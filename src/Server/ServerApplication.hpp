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
	class Player;

	class ServerApplication final : public BaseApplication
	{
		friend GameWorker;

		public:
			using ServerCallback = std::function<void()>;
			using WorkerFunction = std::function<void()>;

			ServerApplication();
			virtual ~ServerApplication();

			inline void DispatchWork(WorkerFunction workFunc);

			inline Database& GetGlobalDatabase();
			inline CollisionMeshStore& GetCollisionMeshStore();
			inline const CollisionMeshStore& GetCollisionMeshStore() const;
			inline ModuleStore& GetModuleStore();
			inline const ModuleStore& GetModuleStore() const;
			inline const NetworkStringStore& GetNetworkStringStore() const;
			inline SpaceshipHullStore& GetSpaceshipHullStore();
			inline const SpaceshipHullStore& GetSpaceshipHullStore() const;

			bool LoadDatabase();

			bool Run() override;

			void HandleCreateSpaceship(std::size_t peerId, const Packets::CreateSpaceship& data);
			void HandleDeleteSpaceship(std::size_t peerId, const Packets::DeleteSpaceship& data);
			void HandleLogin(std::size_t peerId, const Packets::Login& data);
			void HandleJoinArena(std::size_t peerId, const Packets::JoinArena& data);
			void HandlePlayerChat(std::size_t peerId, const Packets::PlayerChat& data);
			void HandlePlayerMovement(std::size_t peerId, const Packets::PlayerMovement& data);
			void HandlePlayerShoot(std::size_t peerId, const Packets::PlayerShoot& data);
			void HandleRegister(std::size_t peerId, const Packets::Register& data);
			void HandleSpawnSpaceship(std::size_t peerId, const Packets::SpawnSpaceship& data);
			void HandleTimeSyncRequest(std::size_t peerId, const Packets::TimeSyncRequest& data);

			inline void RegisterCallback(ServerCallback callback);

		private:
			using CallbackQueue = moodycamel::ConcurrentQueue<ServerCallback>;
			using WorkerQueue = moodycamel::BlockingConcurrentQueue<WorkerFunction>;

			inline WorkerQueue& GetWorkerQueue();

			void HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data) override;
			void HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data) override;
			void HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet) override;

			void InitGameWorkers(std::size_t workerCount);
			void InitGlobalDatabase(std::size_t workerCount, std::string dbHost, Nz::UInt16 port, std::string dbUser, std::string dbPassword, std::string dbName);

			void OnConfigLoaded(const ConfigFile& config) override;

			void RegisterConfigOptions();
			void RegisterNetworkedStrings();

			std::vector<Player*> m_players;
			std::vector<std::unique_ptr<Arena>> m_arenas;
			Nz::MemoryPool m_playerPool;
			CallbackQueue m_callbackQueue;
			CollisionMeshStore m_collisionMeshStore;
			ModuleStore m_moduleStore;
			NetworkStringStore m_stringStore;
			ServerChatCommandStore m_chatCommandStore;
			SpaceshipHullStore m_spaceshipHullStore;
			VisualMeshStore m_visualMeshStore;
			WorkerQueue m_workerQueue;
			std::optional<GlobalDatabase> m_globalDatabase;
			std::vector<std::unique_ptr<GameWorker>> m_workers;
			ServerCommandStore m_commandStore;
	};
}

#include <Server/ServerApplication.inl>

#endif // EREWHON_SERVER_APPLICATION_HPP
