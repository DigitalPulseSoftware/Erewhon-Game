// Copyright (C) 2018 Jérôme Leclercq
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
			PrepareStatement(conn, "AddSpaceshipModule", "INSERT INTO spaceship_modules(spaceship_id, module_id) VALUES($1, $2)", { DatabaseType::Int32, DatabaseType::Int32 });
			PrepareStatement(conn, "CreateSpaceship", "INSERT INTO spaceships(name, script, owner_id, spaceship_hull_id, last_update_date) VALUES(LOWER($2), $3, $1, $4, NOW()) RETURNING id;", { DatabaseType::Int32, DatabaseType::Text, DatabaseType::Text, DatabaseType::Int32 });
			PrepareStatement(conn, "DeleteSpaceship", "DELETE FROM spaceships WHERE owner_id = $1 AND name = LOWER($2)", { DatabaseType::Int32, DatabaseType::Text });
			PrepareStatement(conn, "FindAccountByLogin", "SELECT id, password, password_salt FROM accounts WHERE login=LOWER($1);", { DatabaseType::Text });
			PrepareStatement(conn, "FindSpaceshipModulesBySpaceshipId", "SELECT module_id FROM spaceship_modules WHERE spaceship_id = $1", { DatabaseType::Int32 });
			PrepareStatement(conn, "FindSpaceshipByOwnerIdAndName", "SELECT id, script, spaceship_hull_id FROM spaceships WHERE owner_id = $1 AND name=LOWER($2);", { DatabaseType::Int32, DatabaseType::Text });
			PrepareStatement(conn, "LoadAccount", "SELECT login, display_name, permission_level FROM accounts WHERE id=$1;", { DatabaseType::Int32 });
			PrepareStatement(conn, "LoadCollisionMeshes", "SELECT id, file_path FROM collision_meshes ORDER BY id ASC", {});
			PrepareStatement(conn, "LoadModules", "SELECT id, name, description, class_name, class_info FROM modules ORDER BY id ASC", {});
			PrepareStatement(conn, "LoadSpaceshipHulls", "SELECT id, name, description, collision_mesh FROM spaceship_hulls ORDER BY id ASC", {});
			PrepareStatement(conn, "LoadVisualMeshes", "SELECT id, file_path FROM visual_meshes ORDER BY id ASC", {});
			PrepareStatement(conn, "RegisterAccount", "INSERT INTO accounts(login, display_name, password, password_salt, email, creation_date) VALUES (LOWER($1), $1, $2, $3, $4, NOW());", { DatabaseType::Text, DatabaseType::Text, DatabaseType::Text, DatabaseType::Text });
			PrepareStatement(conn, "UpdateLastLoginDate", "UPDATE accounts SET last_login_date=NOW() WHERE id=$1", { DatabaseType::Int32 });
			PrepareStatement(conn, "UpdatePermissionLevel", "UPDATE accounts SET permission_level=$2 WHERE id=$1", { DatabaseType::Int32, DatabaseType::Int16 });
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to prepare statements: " << e.what() << std::endl;
			throw;
		}
	}
}
