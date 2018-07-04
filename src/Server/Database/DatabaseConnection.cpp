// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Database/DatabaseConnection.hpp>
#include <Nazara/Core/StackArray.hpp>
#include <Nazara/Network/Algorithm.hpp>
#include <Shared/Utils.hpp>
#include <Server/Database/DatabaseResult.hpp>
#include <json/json.hpp>
#include <postgresql/libpq-fe.h>
#include <array>
#include <cassert>
#include <cstring>

namespace ewn
{
	DatabaseConnection::DatabaseConnection(const std::string& dbHost, const std::string& port, const std::string& dbUser, const std::string& dbPassword, const std::string& dbName)
	{
		constexpr std::size_t parameterCount = 14;

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
		AddParameter("keepalives", "1");
		AddParameter("keepalives_idle", "30");
		AddParameter("keepalives_interval", "5");
		AddParameter("keepalives_count", "6");

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
		return ExecPreparedStatement(statementName, &*parameters.begin(), parameters.size());
	}

	DatabaseResult DatabaseConnection::ExecPreparedStatement(const std::string& statementName, const std::vector<DatabaseValue>& parameters)
	{
		return ExecPreparedStatement(statementName, parameters.data(), parameters.size());
	}

	DatabaseResult DatabaseConnection::ExecPreparedStatement(const std::string& statementName, const DatabaseValue* parameters, std::size_t parameterCount)
	{
		Nz::StackArray<const char*> parameterValues = NazaraStackArrayNoInit(const char*, parameterCount);
		Nz::StackArray<int> parameterSize = NazaraStackArrayNoInit(int, parameterCount);
		Nz::StackArray<int> parameterFormat = NazaraStackArrayNoInit(int, parameterCount);

		Nz::Int8 boolTrue = 1;
		Nz::Int8 boolFalse = 0;

		// Allocate a raw memory array to store temporary representations of types
		std::size_t memSize = 0;
		for (std::size_t i = 0; i < parameterCount; ++i)
		{
			std::visit([&](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;

				if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, char> || std::is_same_v<T, const char*> ||
				              std::is_same_v<T, std::string> || std::is_same_v<T, std::vector<Nz::UInt8>>)
				{
					// Nothing to do
				}
				else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double> ||
				                   std::is_same_v<T, Nz::Int16> || std::is_same_v<T, Nz::Int32> ||
				                   std::is_same_v<T, Nz::Int64>)
				{
					// Primitives types requiring big endian representation
					memSize += sizeof(T);
				}
				else if constexpr (std::is_same_v<T, nlohmann::json>)
				{
					//FIXME: Dump JSon only once
					memSize += arg.dump().size();
				}
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

			}, parameters[i]);
		}

		Nz::StackArray<Nz::UInt8> internalRepresentations = NazaraStackArrayNoInit(Nz::UInt8, memSize);
		std::size_t internalRepresentationOffset = 0;

		for (std::size_t i = 0; i < parameterCount; ++i)
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
					valuePtr = &arg;
					valueSize = sizeof(char);
				}
				else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double> ||
				                   std::is_same_v<T, Nz::Int16> || std::is_same_v<T, Nz::Int32> ||
				                   std::is_same_v<T, Nz::Int64>)
				{
					void* bigEndianPtr = &internalRepresentations[internalRepresentationOffset];

					valuePtr = bigEndianPtr;
					valueSize = sizeof(T);

					T bigEndianValue = Nz::HostToNet(arg);
					std::memcpy(bigEndianPtr, &bigEndianValue, sizeof(bigEndianValue));
					internalRepresentationOffset += valueSize;
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
				else if constexpr (std::is_same_v<T, nlohmann::json>)
				{
					std::string jsonDump = arg.dump();
					void* internalPtr = &internalRepresentations[internalRepresentationOffset];
					std::memcpy(internalPtr, jsonDump.data(), jsonDump.size());

					valuePtr = internalPtr;
					valueSize = jsonDump.size();

					internalRepresentationOffset += jsonDump.size();
				}
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

				parameterSize[i] = int(valueSize);
				parameterValues[i] = static_cast<const char*>(valuePtr);

			}, parameters[i]);
		}

		parameterFormat.fill(1); //< Push everything as binary

		return DatabaseResult(PQexecPrepared(m_connection, statementName.data(), int(parameterCount), parameterValues.data(), parameterSize.data(), parameterFormat.data(), 1));
	}

	std::string DatabaseConnection::GetLastErrorMessage() const
	{
		return PQerrorMessage(m_connection);
	}

	bool DatabaseConnection::IsConnected() const
	{
		return PQstatus(m_connection) == CONNECTION_OK;
	}

	bool DatabaseConnection::IsInTransaction() const
	{
		switch (PQtransactionStatus(m_connection))
		{
			case PQTRANS_INERROR:
			case PQTRANS_INTRANS:
			return true;

			case PQTRANS_IDLE:
			case PQTRANS_ACTIVE:
			case PQTRANS_UNKNOWN:
			default:
				return false;
		}
	}

	ewn::DatabaseResult DatabaseConnection::PrepareStatement(const std::string& statementName, const std::string& query, std::initializer_list<DatabaseType> parameterTypes)
	{
		Nz::StackArray<Oid> parameterIds = NazaraStackArrayNoInit(Oid, parameterTypes.size());

		auto parameterId = parameterTypes.begin();
		for (std::size_t i = 0; i < parameterTypes.size(); ++i)
			parameterIds[i] = GetDatabaseOid(*parameterId++);

		return DatabaseResult(PQprepare(m_connection, statementName.data(), query.data(), int(parameterIds.size()), parameterIds.data()));
	}
}
