// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_BROADCASTSYSTEM_HPP
#define EREWHON_SERVER_BROADCASTSYSTEM_HPP

#include <NDK/EntityList.hpp>
#include <NDK/System.hpp>
#include <Shared/Protocol/Packets.hpp>

namespace ewn
{
	class ServerApplication;

	class BroadcastSystem : public Ndk::System<BroadcastSystem>
	{
		public:
			BroadcastSystem(ServerApplication* app);
			~BroadcastSystem() = default;

			void BuildCreateEntity(Ndk::Entity* entity, Packets::CreateEntity& createPacket);
			void CreateAllEntities(std::vector<Packets::CreateEntity>& packetVector);

			NazaraSignal(BroadcastEntityCreation, const BroadcastSystem*, const Packets::CreateEntity& /*packet*/);
			NazaraSignal(BroadcastEntityDestruction, const BroadcastSystem*, const Packets::DeleteEntity& /*packet*/);
			NazaraSignal(BroadcastStateUpdate, const BroadcastSystem*, Packets::ArenaState& /*statePacket*/);

			static Ndk::SystemIndex systemIndex;

		private:
			void OnEntityRemoved(Ndk::Entity* entity) override;
			void OnEntityValidation(Ndk::Entity* entity, bool justAdded) override;
			void OnUpdate(float elapsedTime) override;

			Ndk::EntityList m_movingEntities;
			Nz::UInt16 m_snapshotId;
			Packets::ArenaState m_arenaStatePacket;
			ServerApplication* m_app;
			float m_stateUpdateAccumulator;
			float m_stateUpdateFrequency;
	};
}

#include <Server/Systems/BroadcastSystem.inl>

#endif // EREWHON_SERVER_ARENA_HPP
