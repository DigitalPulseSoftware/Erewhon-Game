// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_DATABASESTORE_HPP
#define EREWHON_SERVER_DATABASESTORE_HPP

#include <functional>
#include <string>

namespace ewn
{
	class Database;
	class DatabaseResult;
	class ServerApplication;

	class DatabaseStore
	{
		friend class DatabaseLoader;

		public:
			virtual ~DatabaseStore();

			inline bool IsLoaded() const;

			void LoadFromDatabase(ServerApplication* app, Database& database, std::function<void(bool success)> callback = nullptr);

			void QueryDatabase(Database& database, std::function<void(DatabaseResult&& result)> callback = nullptr);

			static constexpr std::size_t InvalidEntryId = std::numeric_limits<std::size_t>::max();

		protected:
			inline DatabaseStore(std::string query);

		private:
			virtual bool FillStore(ServerApplication* app, DatabaseResult& result) = 0;
			inline bool FillStoreFromDatabase(ServerApplication* app, DatabaseResult& result);

			std::string m_query;
			bool m_isLoaded;
	};
}

#include <Server/DatabaseStore.inl>

#endif // EREWHON_SERVER_DATABASESTORE_HPP
