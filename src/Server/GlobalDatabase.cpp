// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/GlobalDatabase.hpp>

namespace ewn
{
	void GlobalDatabase::PrepareStatements(DatabaseConnection& conn)
	{
		PrepareStatement(conn, "FindAccountByLogin", "SELECT password FROM account WHERE login=LOWER($1)", { ewn::DatabaseType::Text });
	}
}
