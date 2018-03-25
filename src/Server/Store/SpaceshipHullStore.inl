// Copyright (C) 2017 Jérôme Leclercq
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

	inline std::size_t SpaceshipHullStore::GetEntryCollisionMeshId(std::size_t entryId) const
	{
		assert(IsEntryLoaded(entryId));
		return m_hullInfos[entryId].collisionMeshId;
	}

	inline bool SpaceshipHullStore::IsEntryLoaded(std::size_t entryId) const
	{
		assert(entryId < m_hullInfos.size());
		return m_hullInfos[entryId].isLoaded;
	}
}
