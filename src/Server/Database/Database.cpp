// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Database/Database.hpp>

namespace ewn
{
	DatabaseConnection Database::CreateConnection()
	{
		return DatabaseConnection(m_dbHostname, std::to_string(m_dbPort), m_dbUsername, m_dbPassword, m_dbName);
	}
}
