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
			PrepareStatement<Accounts_QueryConnectionInfoByLogin>(conn);
			PrepareStatement<Accounts_SelectById>(conn);
			PrepareStatement<CollisionMeshes_Load>(conn);
			PrepareStatement<Fleet_Delete>(conn);
			PrepareStatement(conn, "AddSpaceshipModule", "INSERT INTO spaceship_modules(spaceship_id, module_id) VALUES($1, $2)", { DatabaseType::Int32, DatabaseType::Int32 });
			PrepareStatement(conn, "CountFleetByOwnerIdExceptName", "SELECT COUNT(id) FROM spaceships WHERE owner_id = $1 AND name <> LOWER($2)", { DatabaseType::Int32 });
			PrepareStatement(conn, "CountSpaceshipByOwnerIdExceptName", "SELECT COUNT(id) FROM spaceships WHERE owner_id = $1 AND name <> LOWER($2)", { DatabaseType::Int32 });
			PrepareStatement(conn, "CreateAccountToken", "INSERT INTO account_tokens(account_id, token) VALUES($1, $2)", { DatabaseType::Int32, DatabaseType::Text });
			PrepareStatement(conn, "CreateFleet", "INSERT INTO fleets(owner_id, name, last_update_date) VALUES($1, LOWER($2), NOW()) RETURNING id;", { DatabaseType::Int32, DatabaseType::Text });
			PrepareStatement(conn, "CreateFleetSpaceship", "INSERT INTO fleet_spaceships(fleet_id, spaceship_id, position_x, position_y, position_z) VALUES($1, $2, $3, $4, $5)", { DatabaseType::Int32, DatabaseType::Int32, DatabaseType::Single, DatabaseType::Single, DatabaseType::Single });
			PrepareStatement(conn, "CreateSpaceship", "INSERT INTO spaceships(name, script, owner_id, spaceship_hull_id, last_update_date) VALUES(LOWER($2), $3, $1, $4, NOW()) RETURNING id;", { DatabaseType::Int32, DatabaseType::Text, DatabaseType::Text, DatabaseType::Int32 });
			PrepareStatement(conn, "DeleteAccountTokenByAccountId", "DELETE FROM account_tokens WHERE account_id = $1", { DatabaseType::Int32 });
			//PrepareStatement(conn, "DeleteFleet", "DELETE FROM fleets WHERE owner_id = $1 AND name = LOWER($2)", { DatabaseType::Int32, DatabaseType::Text });
			PrepareStatement(conn, "DeleteFleetSpaceships", "DELETE FROM fleet_spaceships WHERE fleet_id = $1", { DatabaseType::Int32 });
			PrepareStatement(conn, "DeleteSpaceship", "DELETE FROM spaceships WHERE owner_id = $1 AND name = LOWER($2)", { DatabaseType::Int32, DatabaseType::Text });
			//PrepareStatement(conn, "FindAccountByLogin", "SELECT id, password, password_salt FROM accounts WHERE login=LOWER($1)", { DatabaseType::Text });
			PrepareStatement(conn, "FindAccountByToken", "SELECT account_id FROM account_tokens WHERE token=$1", { DatabaseType::Text });
			PrepareStatement(conn, "FindFleetByOwnerIdAndName", "SELECT id FROM fleets WHERE owner_id = $1 AND name=LOWER($2)", { DatabaseType::Int32, DatabaseType::Text });
			PrepareStatement(conn, "FindFleetSpaceshipsByFleetId", "SELECT spaceship_id, position_x, position_y, position_z FROM fleet_spaceships WHERE fleet_id = $1", { DatabaseType::Int32 });
			PrepareStatement(conn, "FindFleetsByOwnerId", "SELECT id, name FROM fleets WHERE owner_id = $1", { DatabaseType::Int32 });
			PrepareStatement(conn, "FindSpaceshipByOwnerIdAndName", "SELECT id, script, spaceship_hull_id FROM spaceships WHERE owner_id = $1 AND name=LOWER($2)", { DatabaseType::Int32, DatabaseType::Text });
			PrepareStatement(conn, "FindSpaceshipById", "SELECT name, script, spaceship_hull_id FROM spaceships WHERE id = $1", { DatabaseType::Int32 });
			PrepareStatement(conn, "FindSpaceshipByIdAndOwnerId", "SELECT name, script, spaceship_hull_id FROM spaceships WHERE id = $1 AND owner_id=$2", { DatabaseType::Int32, DatabaseType::Int32 });
			PrepareStatement(conn, "FindSpaceshipModulesBySpaceshipId", "SELECT module_id FROM spaceship_modules WHERE spaceship_id = $1", { DatabaseType::Int32 });
			PrepareStatement(conn, "FindSpaceshipIdByOwnerIdAndName", "SELECT id FROM spaceships WHERE owner_id = $1 AND name=LOWER($2)", { DatabaseType::Int32, DatabaseType::Text });
			PrepareStatement(conn, "FindSpaceshipsByOwnerId", "SELECT id, name FROM spaceships WHERE owner_id = $1", { DatabaseType::Int32 });
			//PrepareStatement(conn, "LoadAccount", "SELECT login, display_name, permission_level FROM accounts WHERE id=$1;", { DatabaseType::Int32 });
			//PrepareStatement(conn, "LoadCollisionMeshes", "SELECT id, file_path, scale FROM collision_meshes ORDER BY id ASC", {});
			PrepareStatement(conn, "LoadModules", "SELECT id, name, description, class_name, class_info, type FROM modules ORDER BY id ASC", {});
			PrepareStatement(conn, "LoadSpaceshipHulls", "SELECT id, name, description, collision_mesh, visual_mesh FROM spaceship_hulls ORDER BY id ASC", {});
			PrepareStatement(conn, "LoadSpaceshipHullSlots", "SELECT module_type FROM spaceship_hull_slots WHERE spaceship_hull_id = $1", { DatabaseType::Int32 });
			PrepareStatement(conn, "LoadVisualMeshes", "SELECT id, file_path FROM visual_meshes ORDER BY id ASC", {});
			PrepareStatement(conn, "Ping", "SELECT 1", {});
			PrepareStatement(conn, "RegisterAccount", "INSERT INTO accounts(login, display_name, password, password_salt, email, creation_date) VALUES (LOWER($1), $1, $2, $3, $4, NOW())", { DatabaseType::Text, DatabaseType::Text, DatabaseType::Text, DatabaseType::Text });
			PrepareStatement(conn, "UpdateFleetNameById", "UPDATE fleets SET name=LOWER($2) WHERE id=$1", { DatabaseType::Int32, DatabaseType::Text });
			PrepareStatement(conn, "UpdateFleetUpdateDate", "UPDATE fleets SET last_update_date=NOW() WHERE id=$1", { DatabaseType::Int32 });
			PrepareStatement(conn, "UpdateLastLoginDate", "UPDATE accounts SET last_login_date=NOW() WHERE id=$1", { DatabaseType::Int32 });
			PrepareStatement(conn, "UpdatePermissionLevel", "UPDATE accounts SET permission_level=$2 WHERE id=$1", { DatabaseType::Int32, DatabaseType::Int16 });
			PrepareStatement(conn, "UpdateSpaceshipModule", "UPDATE spaceship_modules SET module_id=$3 WHERE spaceship_id=$1 AND module_id=$2", { DatabaseType::Int32, DatabaseType::Int32, DatabaseType::Int32 });
			PrepareStatement(conn, "UpdateSpaceshipNameById", "UPDATE spaceships SET name=LOWER($2) WHERE id=$1", { DatabaseType::Int32, DatabaseType::Text });
			PrepareStatement(conn, "UpdateSpaceshipScriptById", "UPDATE spaceships SET script=$2 WHERE id=$1", { DatabaseType::Int32, DatabaseType::Text });
			PrepareStatement(conn, "UpdateSpaceshipUpdateDate", "UPDATE spaceships SET last_update_date=NOW() WHERE id=$1", { DatabaseType::Int32 });
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to prepare statements: " << e.what() << std::endl;
			throw;
		}
	}
}
