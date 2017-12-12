// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_SERVERMATCHENTITIES_HPP
#define EREWHON_CLIENT_SERVERMATCHENTITIES_HPP

#include <Nazara/Core/Signal.hpp>
#include <NDK/World.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Client/ServerConnection.hpp>
#include <vector>

namespace ewn
{
	class ServerMatchEntities
	{
		public:
			struct ServerEntity;

			inline ServerMatchEntities(ServerConnection* server, Ndk::WorldHandle world);
			ServerMatchEntities(const ServerMatchEntities&) = delete;
			ServerMatchEntities(ServerMatchEntities&&) = delete;
			~ServerMatchEntities();

			inline ServerEntity& GetServerEntity(std::size_t id);
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
			};

			NazaraSignal(OnEntityCreated, ServerMatchEntities* /*emitter*/, ServerEntity& /*entity*/);
			NazaraSignal(OnEntityDelete,  ServerMatchEntities* /*emitter*/, ServerEntity& /*entity*/);

		private:
			struct Snapshot;

			void CreateEntityTemplates();
			inline ServerEntity& CreateServerEntity(std::size_t id);

			void OnArenaState(ServerConnection* server, const Packets::ArenaState& arenaState);
			void OnCreateEntity(ServerConnection* server, const Packets::CreateEntity& createPacket);
			void OnDeleteEntity(ServerConnection* server, const Packets::DeleteEntity& deletePacket);

			void ApplySnapshot(const Snapshot& snapshot);

			void CopyState(std::size_t index, const Packets::ArenaState& arenaState);

			bool HandleNextSnapshot();

			void MarkStateAsLost(std::size_t first, std::size_t last);

			void ResetSnapshots(const Packets::ArenaState& arenaState);

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

				Nz::UInt16 stateId;
				std::vector<Entity> entities;
				bool isValid; //< False if server hasn't send it yet (meaning we could have missed it)
			};

			NazaraSlot(ServerConnection, OnArenaState,   m_onArenaStateSlot);
			NazaraSlot(ServerConnection, OnCreateEntity, m_onCreateEntitySlot);
			NazaraSlot(ServerConnection, OnDeleteEntity, m_onDeleteEntitySlot);

			std::size_t m_jitterBufferSize;
			std::vector<ServerEntity> m_serverEntities;
			std::vector<Snapshot> m_jitterBuffer;
			Ndk::EntityHandle m_ballTemplateEntity;
			Ndk::EntityHandle m_earthTemplateEntity;
			Ndk::EntityHandle m_debugTemplateEntity;
			Ndk::EntityHandle m_spaceshipTemplateEntity;
			Ndk::WorldHandle m_world;
			ServerConnection* m_server;
			float m_correctionAccumulator;
			float m_snapshotUpdateAccumulator;
	};
}

#include <Client/ServerMatchEntities.inl>

#endif // EREWHON_CLIENT_SERVERMATCHENTITIES_HPP
