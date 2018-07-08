// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_DATABASECONNECTION_HPP
#define EREWHON_SERVER_DATABASECONNECTION_HPP

#include <Nazara/Prerequisites.hpp>
#include <Nazara/Core/MovablePtr.hpp>
#include <Server/Database/DatabaseResult.hpp>
#include <Server/Database/DatabaseTypes.hpp>
#include <string>

typedef struct pg_conn PGconn;

namespace ewn
{
	class DatabaseConnection
	{
		public:
			DatabaseConnection(const std::string& dbHost, const std::string& port, const std::string& dbUser, const std::string& dbPassword, const std::string& dbName);
			DatabaseConnection(const DatabaseConnection&) = delete;
			DatabaseConnection(DatabaseConnection&&) noexcept = default;
			~DatabaseConnection();

			DatabaseResult Exec(const std::string& query);
			DatabaseResult ExecPreparedStatement(const std::string& statementName, std::initializer_list<DatabaseValue> parameters);
			DatabaseResult ExecPreparedStatement(const std::string& statementName, const std::vector<DatabaseValue>& parameters);
			DatabaseResult ExecPreparedStatement(const std::string& statementName, const DatabaseValue* parameters, std::size_t parameterCount);

			std::string GetLastErrorMessage() const;

			bool IsConnected() const;
			bool IsInTransaction() const;

			DatabaseResult PrepareStatement(const std::string& statementName, const std::string& query, std::initializer_list<DatabaseType> parameterTypes);
			DatabaseResult PrepareStatement(const std::string& statementName, const std::string& query, const DatabaseType* parameterTypes, std::size_t typeCount);

			DatabaseConnection& operator=(const DatabaseConnection&) = delete;
			DatabaseConnection& operator=(DatabaseConnection&&) noexcept = default;

		private:
			Nz::MovablePtr<PGconn> m_connection;
	};
}

#include <Server/Database/DatabaseConnection.inl>

#endif // EREWHON_SERVER_DATABASECONNECTION_HPP
