// Copyright (C) 2018 Jérôme Leclercq
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

	inline const Nz::Collider3DRef& CollisionMeshStore::GetEntryCollider(std::size_t entryId) const
	{
		assert(IsEntryLoaded(entryId));
		return m_collisionInfos[entryId].collider;
	}

	inline const Nz::Boxf& CollisionMeshStore::GetEntryDimensions(std::size_t entryId) const
	{
		assert(IsEntryLoaded(entryId));
		return m_collisionInfos[entryId].dimensions;
	}

	inline std::size_t CollisionMeshStore::GetEntryCount() const
	{
		return m_collisionInfos.size();
	}

	inline const std::string& CollisionMeshStore::GetEntryFilePath(std::size_t entryId) const
	{
		assert(IsEntryLoaded(entryId));
		return m_collisionInfos[entryId].filePath;
	}

	inline bool CollisionMeshStore::IsEntryLoaded(std::size_t entryId) const
	{
		assert(entryId < m_collisionInfos.size());
		return m_collisionInfos[entryId].isLoaded;
	}
}
