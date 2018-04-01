// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_SERVERMATCHENTITIES_HPP
#define EREWHON_CLIENT_SERVERMATCHENTITIES_HPP

#include <Nazara/Core/Signal.hpp>
#include <Nazara/Graphics/Material.hpp>
#include <Nazara/Network/UdpSocket.hpp>
#include <NDK/EntityOwner.hpp>
#include <NDK/World.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Client/ServerConnection.hpp>
#include <nonstd/ring_span.hpp>
#include <array>
#include <vector>

namespace ewn
{
	class ClientApplication;

	class ServerMatchEntities
	{
		public:
			struct ServerEntity;

			ServerMatchEntities(ClientApplication* app, ServerConnection* server, Ndk::WorldHandle world);
			ServerMatchEntities(const ServerMatchEntities&) = delete;
			ServerMatchEntities(ServerMatchEntities&&) = delete;
			~ServerMatchEntities();

			inline void EnableSnapshotHandling(bool enable);

			inline ServerEntity& GetServerEntity(std::size_t id);
			inline std::size_t GetServerEntityCount() const;
			inline bool IsSnapshotHandlingEnabled() const;
			inline bool IsServerEntityValid(std::size_t id) const;

			void Update(float elapsedTime);

			ServerMatchEntities& operator=(const ServerMatchEntities&) = delete;
			ServerMatchEntities& operator=(ServerMatchEntities&&) = delete;

			struct ServerEntity
			{
				Ndk::EntityHandle debugGhostEntity;
				Ndk::EntityHandle entity;
				Ndk::EntityHandle textEntity;
				Nz::Quaternionf rotationError;
				Nz::Vector3f positionError;
				Nz::UInt32 serverId;
				bool isValid = false;
				std::string name; //< remove asap, used for temporary client-side radar
			};

			NazaraSignal(OnEntityCreated, ServerMatchEntities* /*emitter*/, ServerEntity& /*entity*/);
			NazaraSignal(OnEntityDelete,  ServerMatchEntities* /*emitter*/, ServerEntity& /*entity*/);

		private:
			struct Snapshot;

			inline ServerEntity& CreateServerEntity(Nz::UInt32 id);
			void FillPrefabFactory();

			void OnArenaModels(ServerConnection* server, const Packets::ArenaModels& arenaModels);
			void OnArenaPrefabs(ServerConnection* server, const Packets::ArenaPrefabs& arenaPrefabs);
			void OnArenaState(ServerConnection* server, const Packets::ArenaState& arenaState);
			void OnCreateEntity(ServerConnection* server, const Packets::CreateEntity& createPacket);
			void OnDeleteEntity(ServerConnection* server, const Packets::DeleteEntity& deletePacket);

			void ApplySnapshot(const Snapshot& snapshot);

			struct Snapshot
			{
				struct Entity
				{
					Nz::UInt32 id;
					Nz::Vector3f angularVelocity;
					Nz::Vector3f linearVelocity;
					Nz::Vector3f position;
					Nz::Quaternionf rotation;
				};

				Nz::UInt64 applyTime;
				Nz::UInt16 stateId;
				std::vector<Entity> entities;
			};

			NazaraSlot(ServerConnection, OnArenaModels,  m_onArenaModelsSlot);
			NazaraSlot(ServerConnection, OnArenaPrefabs, m_onArenaPrefabsSlot);
			NazaraSlot(ServerConnection, OnArenaState,   m_onArenaStateSlot);
			NazaraSlot(ServerConnection, OnCreateEntity, m_onCreateEntitySlot);
			NazaraSlot(ServerConnection, OnDeleteEntity, m_onDeleteEntitySlot);

			using PrefabFactoryFunction = std::function<const Ndk::EntityHandle&(ClientApplication* app, Ndk::World& world)>;

			std::array<Snapshot, 5> m_jitterBufferData;
			nonstd::ring_span<Snapshot> m_jitterBuffer;
			std::unordered_map<std::string, PrefabFactoryFunction> m_prefabFactory;
			std::vector<Ndk::EntityOwner> m_prefabs;
			std::vector<ServerEntity> m_serverEntities;
			Ndk::WorldHandle m_world;
			Nz::UdpSocket m_debugStateSocket;
			Nz::UInt64 m_snapshotDelay;
			ClientApplication* m_app;
			ServerConnection* m_server;
			bool m_stateHandlingEnabled;
			float m_correctionAccumulator;
			float m_snapshotUpdateAccumulator;
	};
}

#include <Client/ServerMatchEntities.inl>

#endif // EREWHON_CLIENT_SERVERMATCHENTITIES_HPP
