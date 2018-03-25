// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/DatabaseStore.hpp>
#include <Server/Database/Database.hpp>
#include <Server/Database/DatabaseResult.hpp>
#include <iostream>

namespace ewn
{
	DatabaseStore::~DatabaseStore() = default;

	void DatabaseStore::LoadFromDatabase(ServerApplication* app, Database& database, std::function<void(bool success)> callback)
	{
		database.ExecuteQuery(m_query, {}, [this, app, cb = std::move(callback)](DatabaseResult& result)
		{
			if (result)
				cb(FillStore(app, result));
			else
			{
				std::cerr << "An error occurred on prepared statement " << m_query << ": " << result.GetLastErrorMessage() << std::endl;
				cb(false);
				return;
			}
		});
	}

	void DatabaseStore::QueryDatabase(Database& database, std::function<void(DatabaseResult&& result)> callback)
	{
		database.ExecuteQuery(m_query, {}, [cb = std::move(callback)](DatabaseResult& result)
		{
			cb(std::move(result));
		});
	}
}
