// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_SERVERMATCHENTITIES_HPP
#define EREWHON_CLIENT_SERVERMATCHENTITIES_HPP

#include <NDK/World.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Client/ServerConnection.hpp>
#include <vector>

namespace ewn
{
	class ServerMatchEntities
	{
		public:
			struct Snapshot;

			inline ServerMatchEntities(ServerConnection* server, Ndk::WorldHandle world);
			ServerMatchEntities(const ServerMatchEntities&) = delete;
			ServerMatchEntities(ServerMatchEntities&&) = delete;
			~ServerMatchEntities() = default;

			template<typename T> bool HandleSnapshot(T&& callback);

			ServerMatchEntities& operator=(const ServerMatchEntities&) = delete;
			ServerMatchEntities& operator=(ServerMatchEntities&&) = delete;

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

		private:
			struct ServerEntity;

			void CreateEntityTemplates();
			inline ServerEntity& CreateServerEntity(std::size_t id);
			inline ServerEntity& GetServerEntity(std::size_t id);
			inline bool IsServerEntityValid(std::size_t id) const;

			void OnArenaState(ServerConnection* server, const Packets::ArenaState& arenaState);
			void OnCreateEntity(ServerConnection* server, const Packets::CreateEntity& createPacket);
			void OnDeleteEntity(ServerConnection* server, const Packets::DeleteEntity& deletePacket);

			void CopyState(std::size_t index, const Packets::ArenaState& arenaState);

			void MarkStateAsLost(std::size_t first, std::size_t last);

			void ResetSnapshots(const Packets::ArenaState& arenaState);

			struct ServerEntity
			{
				Ndk::EntityHandle debugGhostEntity;
				Ndk::EntityHandle entity;
				Ndk::EntityHandle textEntity;
				bool isValid = false;
			};

			NazaraSlot(ServerConnection, OnArenaState, m_onArenaStateSlot);

			std::size_t m_jitterBufferSize;
			std::vector<ServerEntity> m_serverEntities;
			std::vector<Snapshot> m_jitterBuffer;
			Ndk::EntityHandle m_ballTemplateEntity;
			Ndk::EntityHandle m_earthTemplateEntity;
			Ndk::EntityHandle m_debugTemplateEntity;
			Ndk::EntityHandle m_spaceshipTemplateEntity;
			Ndk::WorldHandle m_world;
			ServerConnection* m_server;
	};
}

#include <Client/ServerMatchEntities.inl>

#endif // EREWHON_CLIENT_SERVERMATCHENTITIES_HPP
