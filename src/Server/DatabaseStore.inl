// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/DatabaseStore.hpp>
#include <cassert>

namespace ewn
{
	inline DatabaseStore::DatabaseStore(std::string query) :
	m_query(std::move(query)),
	m_isLoaded(false)
	{
	}

	inline bool DatabaseStore::IsLoaded() const
	{
		return m_isLoaded;
	}

	inline bool DatabaseStore::FillStoreFromDatabase(ServerApplication* app, DatabaseResult& result)
	{
		m_isLoaded = FillStore(app, result);
		return IsLoaded();
	}
}
