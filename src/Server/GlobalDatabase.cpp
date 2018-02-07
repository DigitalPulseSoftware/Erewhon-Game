// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/GlobalDatabase.hpp>
#include <iostream>

namespace ewn
{
	void GlobalDatabase::PrepareStatements(DatabaseConnection& conn)
	{
		try
		{
			PrepareStatement(conn, "FindAccountByLogin", "SELECT password, password_salt FROM account WHERE login=LOWER($1);", { DatabaseType::Text });
			PrepareStatement(conn, "RegisterAccount", "INSERT INTO account(login, display_name, password, password_salt, email, creation_date) VALUES (LOWER($1), $1, $2, $3, $4, NOW());", { DatabaseType::Text, DatabaseType::Text, DatabaseType::Text, DatabaseType::Text });
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to prepare statements: " << e.what() << std::endl;
			throw;
		}
	}
}
