// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/DatabaseLoader.hpp>
#include <cassert>

namespace ewn
{
	inline void DatabaseLoader::RegisterStore(std::string name, DatabaseStore* store, std::vector<std::string> dependencies)
	{
		StoreData& newStore = m_stores.emplace_back();
		newStore.dependencies = std::move(dependencies);
		newStore.store = store;
		newStore.storeName = std::move(name);

		assert(std::find(newStore.dependencies.begin(), newStore.dependencies.end(), newStore.storeName) == newStore.dependencies.end());
	}
}
