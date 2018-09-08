// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_GLOBALDATABASE_HPP
#define EREWHON_SERVER_GLOBALDATABASE_HPP

#include <Server/Database/Database.hpp>

namespace ewn
{
	class GlobalDatabase final : public Database
	{
		friend DatabaseWorker;

		public:
			inline GlobalDatabase(std::string dbHost, Nz::UInt16 port, std::string dbUser, std::string dbPassword, std::string dbName);
			~GlobalDatabase() = default;

		private:
			void PrepareStatements(DatabaseConnection& conn) override;
	};

	struct Accounts_QueryConnectionInfoByLogin : PreparedStatement<Accounts_QueryConnectionInfoByLogin>
	{
		std::string login;

		void FillParameters(std::vector<DatabaseValue>& values)
		{
			values.emplace_back(std::move(login));
		}

		struct Result
		{
			Nz::Int32 id;
			std::string password;
			std::string salt;

			Result(DatabaseResult& result)
			{
				id = std::get<Nz::Int32>(result.GetValue(0));
				password = std::get<std::string>(result.GetValue(1));
				salt = std::get<std::string>(result.GetValue(2));
			}
		};

		static constexpr const char* StatementName = "Account_QueryConnectionDataByLogin";
		static constexpr const char* Query = "SELECT id, password, password_salt FROM accounts WHERE login=LOWER($1)";
		static constexpr std::array<DatabaseType, 1> Parameters = { DatabaseType::Text };
	};

	struct Accounts_SelectById : PreparedStatement<Accounts_SelectById>
	{
		Nz::Int32 id;

		void FillParameters(std::vector<DatabaseValue>& values)
		{
			values.emplace_back(id);
		}

		struct Result
		{
			std::string login;
			std::string displayName;
			Nz::Int16 permissionLevel;

			Result(DatabaseResult& result)
			{
				login = std::get<std::string>(result.GetValue(0));
				displayName = std::get<std::string>(result.GetValue(1));
				permissionLevel = std::get<Nz::Int16>(result.GetValue(2));
			}
		};

		static constexpr const char* StatementName = "Accounts_SelectById";
		static constexpr const char* Query = "SELECT login, display_name, permission_level FROM accounts WHERE id=$1";
		static constexpr std::array<DatabaseType, 1> Parameters = { DatabaseType::Int32 };
	};

	struct CollisionMeshes_Load : PreparedStatement<CollisionMeshes_Load>
	{
		void FillParameters(std::vector<DatabaseValue>& values)
		{
		}

		struct Result
		{
			Nz::Int32 id;
			std::string filepath;
			float scale;

			Result(DatabaseResult& result, std::size_t rowIndex)
			{
				id = std::get<Nz::Int32>(result.GetValue(0, rowIndex));
				filepath = std::get<std::string>(result.GetValue(1, rowIndex));
				scale = std::get<float>(result.GetValue(2, rowIndex));
			}
		};

		static constexpr const char* StatementName = "CollisionMeshes_Load";
		static constexpr const char* Query = "SELECT id, file_path, scale FROM collision_meshes ORDER BY id ASC";
		static constexpr std::array<DatabaseType, 0> Parameters = {};
	};

	struct Fleet_Delete : PreparedStatement<Fleet_Delete>
	{
		Nz::Int32 ownerId;
		std::string name;

		void FillParameters(std::vector<DatabaseValue>& values)
		{
			values.emplace_back(ownerId);
			values.emplace_back(name);
		}

		static constexpr const char* StatementName = "Fleet_Delete";
		static constexpr const char* Query = "DELETE FROM fleets WHERE owner_id = $1 AND name = LOWER($2)";
		static constexpr std::array<DatabaseType, 2> Parameters = { DatabaseType::Int32, DatabaseType::Text };
	};
}

#include <Server/GlobalDatabase.inl>

#endif // EREWHON_SERVER_GLOBALDATABASE_HPP
