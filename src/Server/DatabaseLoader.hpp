// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_DATABASELOADER_HPP
#define EREWHON_SERVER_DATABASELOADER_HPP

#include <Nazara/Core/Bitset.hpp>
#include <Server/DatabaseStore.hpp>
#include <Server/Database/DatabaseResult.hpp>
#include <string>

namespace ewn
{
	class ServerApplication;

	class DatabaseLoader
	{
		public:
			DatabaseLoader() = default;
			~DatabaseLoader() = default;

			bool LoadFromDatabase(ServerApplication* app, Database& database);

			inline void RegisterStore(std::string name, DatabaseStore* store, std::vector<std::string> dependencies);

		private:
			void ResolveDependencies();

			struct StoreData
			{
				DatabaseResult pendingResult;
				DatabaseStore* store;
				Nz::Bitset<> resolvedDependencies;
				std::string storeName;
				std::vector<std::string> dependencies;
			};

			std::vector<StoreData> m_stores;
			std::vector<std::size_t> m_sortedStore;
			Nz::Bitset<> m_loadedDependencies;
	};
}

#include <Server/DatabaseLoader.inl>

#endif // EREWHON_SERVER_DATABASELOADER_HPP
