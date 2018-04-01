// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/GlobalDatabase.hpp>

namespace ewn
{
	inline GlobalDatabase::GlobalDatabase(std::string dbHost, Nz::UInt16 port, std::string dbUser, std::string dbPassword, std::string dbName) :
	Database("Global", std::move(dbHost), port, std::move(dbUser), std::move(dbPassword), std::move(dbName))
	{
	}
}
