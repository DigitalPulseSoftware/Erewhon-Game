// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_SPACESHIPHULLSTORE_HPP
#define EREWHON_SERVER_SPACESHIPHULLSTORE_HPP

#include <Shared/Enums.hpp>
#include <Server/DatabaseStore.hpp>
#include <NDK/Entity.hpp>
#include <string>
#include <vector>

namespace ewn
{
	class Database;
	class DatabaseResult;

	class SpaceshipHullStore final : public DatabaseStore
	{
		public:
			inline SpaceshipHullStore();
			~SpaceshipHullStore() = default;

			inline std::size_t GetEntryCount() const;
			inline std::size_t GetEntryCollisionMeshId(std::size_t entryId) const;
			inline const std::string& GetEntryDescription(std::size_t entryId) const;
			inline const std::string& GetEntryName(std::size_t entryId) const;
			inline std::size_t GetEntrySlotCount(std::size_t entryId) const;
			inline ModuleType GetEntrySlotModuleType(std::size_t entryId, std::size_t slotId) const;
			inline std::size_t GetEntryVisualMeshId(std::size_t entryId) const;

			inline bool IsEntryLoaded(std::size_t entryId) const;

		private:
			bool FillStore(ServerApplication* app, DatabaseResult& result) override;
		
			void LoadSlots(std::size_t hullId, DatabaseResult& result);

			struct SlotInfo
			{
				ModuleType moduleType;
				// Nz::Vector3f position;
			};

			struct HullInfo 
			{
				std::size_t collisionMeshId;
				std::size_t visualMeshId;
				std::string name;
				std::string description;
				std::vector<SlotInfo> slots;
				bool doesExist = false;
				bool isLoaded = false;
			};

			std::vector<HullInfo> m_hullInfos;
			bool m_isLoaded;
	};
}

#include <Server/Store/SpaceshipHullStore.inl>

#endif // EREWHON_SERVER_SPACESHIPHULLSTORE_HPP
