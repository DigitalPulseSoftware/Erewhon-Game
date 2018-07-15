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

	struct Account_QueryConnectionInfoByLogin : PreparedStatement<Account_QueryConnectionInfoByLogin>
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

		static constexpr const char* StatementName = "LoadCollisionMeshes";
		static constexpr const char* Query = "SELECT id, file_path, scale FROM collision_meshes ORDER BY id ASC";
		static constexpr std::array<DatabaseType, 0> Parameters = {};
	};
}

#include <Server/GlobalDatabase.inl>

#endif // EREWHON_SERVER_GLOBALDATABASE_HPP
