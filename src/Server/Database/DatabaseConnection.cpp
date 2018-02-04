// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Database/DatabaseConnection.hpp>
#include <Nazara/Core/MemoryHelper.hpp>
#include <Nazara/Network/Algorithm.hpp>
#include <Server/Database/DatabaseResult.hpp>
#include <pgsql/libpq-fe.h>
#include <array>
#include <cassert>
#include <cstring>

namespace ewn
{
	DatabaseConnection::DatabaseConnection(const std::string& dbHost, const std::string& port, const std::string& dbUser, const std::string& dbPassword, const std::string& dbName)
	{
		constexpr std::size_t parameterCount = 10;

		std::size_t parameterIndex = 0;
		std::array<const char*, parameterCount> keys;
		std::array<const char*, parameterCount> values;

		auto AddParameter = [&](const char* key, const char* value)
		{
			assert(parameterIndex < parameterCount);
			keys[parameterIndex] = key;
			values[parameterIndex] = value;

			parameterIndex++;
		};

		// Fill connection parameters
		AddParameter("host", dbHost.data());
		AddParameter("port", port.data());
		AddParameter("user", dbUser.data());
		AddParameter("password", dbPassword.data());
		AddParameter("dbname", dbName.data());
		AddParameter("sslmode", "require");
		AddParameter("client_encoding", "UTF8");
		AddParameter("connect_timeout", "10");
		AddParameter("application_name", "Utopia-Server");
		AddParameter(nullptr, nullptr); //< End of parameters

		m_connection = PQconnectdbParams(keys.data(), values.data(), 0);
	}

	DatabaseConnection::~DatabaseConnection()
	{
		if (m_connection)
			PQfinish(m_connection);
	}

	DatabaseResult DatabaseConnection::Exec(const std::string& query)
	{
		return DatabaseResult(PQexec(m_connection, query.data()));
	}

	DatabaseResult DatabaseConnection::ExecPreparedStatement(const std::string& statementName, std::initializer_list<DatabaseValue> parameters)
	{
		Nz::StackArray<const char*> parameterValues = NazaraStackAllocationNoInit(const char*, parameters.size());
		Nz::StackArray<int> parameterSize = NazaraStackAllocationNoInit(int, parameters.size());
		Nz::StackArray<int> parameterFormat = NazaraStackAllocationNoInit(int, parameters.size());

		Nz::Int8 boolTrue = 1;
		Nz::Int8 boolFalse = 0;

		std::size_t parameterIndex = 0;
		for (const DatabaseValue& value : parameters)
		{
			std::visit([&](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;

				const void* valuePtr;
				std::size_t valueSize;

				if constexpr (std::is_same_v<T, bool>)
				{
					valuePtr = (arg) ? &boolTrue : &boolFalse;
					valueSize = 1;
				}
				else if constexpr (std::is_same_v<T, char>)
				{
					static_assert(sizeof(char) == 1);

					valuePtr = &arg;
					valueSize = sizeof(char);
				}
				else if constexpr (std::is_same_v<T, float>)
				{
					static_assert(sizeof(float) == 4);

					valuePtr = &arg;
					valueSize = 4;
				}
				else if constexpr (std::is_same_v<T, double>)
				{
					static_assert(sizeof(double) == 8);

					valuePtr = &arg;
					valueSize = 8;
				}
				else if constexpr (std::is_same_v<T, Nz::Int16>)
				{
					valuePtr = &arg;
					valueSize = 2;
				}
				else if constexpr (std::is_same_v<T, Nz::Int32>)
				{
					valuePtr = &arg;
					valueSize = 4;
				}
				else if constexpr (std::is_same_v<T, Nz::Int64>)
				{
					valuePtr = &arg;
					valueSize = 8;
				}
				else if constexpr (std::is_same_v<T, const char*>)
				{
					valuePtr = arg;
					valueSize = std::strlen(arg);
				}
				else if constexpr (std::is_same_v<T, std::string>)
				{
					valuePtr = arg.data();
					valueSize = arg.size();
				}
				else if constexpr (std::is_same_v<T, std::vector<Nz::UInt8>>)
				{
					valuePtr = arg.data();
					valueSize = arg.size();
				}
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

				parameterSize[parameterIndex] = int(valueSize);
				parameterValues[parameterIndex] = static_cast<const char*>(valuePtr);

			}, value);

			parameterIndex++;
		}

		parameterFormat.fill(1); //< Push everything as binary

		return DatabaseResult(PQexecPrepared(m_connection, statementName.data(), int(parameters.size()), parameterValues.data(), parameterSize.data(), parameterFormat.data(), 1));
	}

	DatabaseResult DatabaseConnection::ExecPreparedStatement(const std::string& statementName, const std::vector<DatabaseValue>& parameters)
	{
		return ExecPreparedStatement(statementName, std::initializer_list<DatabaseValue>(parameters.data(), parameters.data() + parameters.size()));
	}

	std::string DatabaseConnection::GetLastErrorMessage() const
	{
		return PQerrorMessage(m_connection);
	}

	bool DatabaseConnection::IsConnected() const
	{
		return PQstatus(m_connection) == CONNECTION_OK;
	}

	ewn::DatabaseResult DatabaseConnection::PrepareStatement(const std::string& statementName, const std::string& query, std::initializer_list<DatabaseType> parameterTypes)
	{
		Nz::StackArray<Oid> parameterIds = NazaraStackAllocationNoInit(Oid, parameterTypes.size());

		auto parameterId = parameterTypes.begin();
		for (std::size_t i = 0; i < parameterTypes.size(); ++i)
			parameterIds[i] = GetDatabaseOid(*parameterId++);

		return DatabaseResult(PQprepare(m_connection, statementName.data(), query.data(), int(parameterIds.size()), parameterIds.data()));
	}
}
