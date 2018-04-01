// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Store/VisualMeshStore.hpp>
#include <cassert>

namespace ewn
{
	inline VisualMeshStore::VisualMeshStore() :
	DatabaseStore("LoadVisualMeshes")
	{
	}

	inline std::size_t VisualMeshStore::GetEntryCount() const
	{
		return m_visualInfos.size();
	}

	inline const std::string& VisualMeshStore::GetEntryFilePath(std::size_t entryId) const
	{
		assert(IsEntryLoaded(entryId));
		return m_visualInfos[entryId].filePath;
	}

	inline bool VisualMeshStore::IsEntryLoaded(std::size_t entryId) const
	{
		assert(entryId < m_visualInfos.size());
		return m_visualInfos[entryId].isLoaded;
	}
}
