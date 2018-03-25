// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/DatabaseLoader.hpp>
#include <Server/Database/Database.hpp>
#include <iostream>
#include <queue>

namespace ewn
{
	bool DatabaseLoader::LoadFromDatabase(ServerApplication* app, Database& database)
	{
		ResolveDependencies();

		for (StoreData& data : m_stores)
		{
			data.store->QueryDatabase(database, [&data](DatabaseResult&& result)
			{
				data.pendingResult = std::move(result);
			});
		}

		database.WaitForCompletion();

		// Check for failed results
		bool hasFailed = false;
		for (StoreData& data : m_stores)
		{
			if (!data.pendingResult)
			{
				std::cerr << "Failed to load " << data.storeName << ": " << data.pendingResult.GetLastErrorMessage() << std::endl;
				hasFailed = true;
			}
		}

		if (hasFailed)
			return false;

		// Load by order of dependencies.
		for (std::size_t storeId : m_sortedStore)
		{
			StoreData& data = m_stores[storeId];
			if (!data.store->FillStoreFromDatabase(app, data.pendingResult))
			{
				std::cerr << "Failed to fill " << data.storeName << " store" << std::endl;
				hasFailed = true;
			}
		}

		return !hasFailed;
	}

	void DatabaseLoader::ResolveDependencies()
	{
		for (StoreData& data : m_stores)
		{
			for (const std::string& dependency : data.dependencies)
			{
				auto it = std::find_if(m_stores.begin(), m_stores.end(), [&dependency](const StoreData& dependencyStore)
				{
					return dependencyStore.storeName == dependency;
				});

				assert(it != m_stores.end() && "Dependency not registered");

				data.resolvedDependencies.UnboundedSet(std::distance(m_stores.begin(), it));
			}
			data.dependencies.clear();
		}

		// Sort store by order of dependencies (Kahn's topological sorting)
		std::vector<std::size_t> dependencyCount(m_stores.size(), 0);
		for (StoreData& data : m_stores)
		{
			for (std::size_t dependencyId = data.resolvedDependencies.FindFirst(); dependencyId != data.resolvedDependencies.npos; dependencyId = data.resolvedDependencies.FindNext(dependencyId))
				dependencyCount[dependencyId]++;
		}

		std::queue<std::size_t> q;
		for (std::size_t i = 0; i < dependencyCount.size(); ++i)
		{
			if (dependencyCount[i] == 0)
				q.push(i);
		}

		std::size_t visitedCount = 0;

		m_sortedStore.clear();
		while (!q.empty())
		{
			std::size_t u = q.front();
			q.pop();

			m_sortedStore.push_back(u);

			StoreData& store = m_stores[u];
			for (std::size_t dependencyId = store.resolvedDependencies.FindFirst(); dependencyId != store.resolvedDependencies.npos; dependencyId = store.resolvedDependencies.FindNext(dependencyId))
				if (--dependencyCount[dependencyId] == 0)
					q.push(dependencyId);

			visitedCount++;
		}

		// Since we gave dependencies instead of "what should be loaded next", we have to reverse the results
		std::reverse(m_sortedStore.begin(), m_sortedStore.end());

		assert(visitedCount == m_stores.size());
	}
}
