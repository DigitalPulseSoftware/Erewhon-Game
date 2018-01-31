// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Database/Database.hpp>

namespace ewn
{
	inline Database::Database(std::string name, std::string dbHost, Nz::UInt16 port, std::string dbUser, std::string dbPassword, std::string dbName) :
	m_name(std::move(name)),
	m_dbHostname(std::move(dbHost)),
	m_dbPort(port),
	m_dbPassword(std::move(dbPassword)),
	m_dbName(std::move(dbName)),
	m_dbUsername(std::move(dbUser))
	{
	}
}
