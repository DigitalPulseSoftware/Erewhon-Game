// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Store/SpaceshipHullStore.hpp>
#include <cassert>

namespace ewn
{
	inline SpaceshipHullStore::SpaceshipHullStore() :
	DatabaseStore("LoadSpaceshipHulls")
	{
	}

	inline std::size_t SpaceshipHullStore::GetEntryCount() const
	{
		return m_hullInfos.size();
	}

	inline std::size_t SpaceshipHullStore::GetEntryCollisionMeshId(std::size_t entryId) const
	{
		assert(IsEntryLoaded(entryId));
		return m_hullInfos[entryId].collisionMeshId;
	}

	inline const std::string& SpaceshipHullStore::GetEntryDescription(std::size_t entryId) const
	{
		assert(IsEntryLoaded(entryId));
		return m_hullInfos[entryId].description;
	}

	inline const std::string& SpaceshipHullStore::GetEntryName(std::size_t entryId) const
	{
		assert(IsEntryLoaded(entryId));
		return m_hullInfos[entryId].name;
	}

	inline std::size_t SpaceshipHullStore::GetEntrySlotCount(std::size_t entryId) const
	{
		assert(IsEntryLoaded(entryId));
		return m_hullInfos[entryId].slots.size();
	}

	inline ModuleType SpaceshipHullStore::GetEntrySlotModuleType(std::size_t entryId, std::size_t slotId) const
	{
		assert(slotId < GetEntrySlotCount(entryId));
		return m_hullInfos[entryId].slots[slotId].moduleType;
	}

	inline std::size_t SpaceshipHullStore::GetEntryVisualMeshId(std::size_t entryId) const
	{
		assert(IsEntryLoaded(entryId));
		return m_hullInfos[entryId].visualMeshId;
	}

	inline bool SpaceshipHullStore::IsEntryLoaded(std::size_t entryId) const
	{
		assert(entryId < m_hullInfos.size());
		return m_hullInfos[entryId].isLoaded;
	}
}
