// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Store/CollisionMeshStore.hpp>
#include <cassert>

namespace ewn
{
	inline CollisionMeshStore::CollisionMeshStore() :
	DatabaseStore("LoadCollisionMeshes")
	{
	}

	inline bool CollisionMeshStore::IsEntryLoaded(std::size_t entryId) const
	{
		assert(entryId < m_collisionInfos.size());
		return m_collisionInfos[entryId].isLoaded;
	}
}
