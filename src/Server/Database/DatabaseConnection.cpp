// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Database/DatabaseConnection.hpp>
#include <Nazara/Core/MemoryHelper.hpp>
#include <Nazara/Network/Algorithm.hpp>
#include <Shared/Utils.hpp>
#include <Server/Database/DatabaseResult.hpp>
#include <postgresql/libpq-fe.h>
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
		return ExecPreparedStatement(statementName, &*parameters.begin(), parameters.size());
	}

	DatabaseResult DatabaseConnection::ExecPreparedStatement(const std::string& statementName, const std::vector<DatabaseValue>& parameters)
	{
		return ExecPreparedStatement(statementName, parameters.data(), parameters.size());
	}

	DatabaseResult DatabaseConnection::ExecPreparedStatement(const std::string& statementName, const DatabaseValue* parameters, std::size_t parameterCount)
	{
		Nz::StackArray<const char*> parameterValues = NazaraStackAllocationNoInit(const char*, parameterCount);
		Nz::StackArray<int> parameterSize = NazaraStackAllocationNoInit(int, parameterCount);
		Nz::StackArray<int> parameterFormat = NazaraStackAllocationNoInit(int, parameterCount);

		Nz::Int8 boolTrue = 1;
		Nz::Int8 boolFalse = 0;

		// Allocate a raw memory array to store temporary big endian representations of types
		Nz::StackArray<Nz::UInt8> bigEndianFormats = NazaraStackAllocationNoInit(Nz::UInt8, 8 * parameterCount);
		std::size_t bigEndianFormatUsedSize = 0;

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
					static_assert(sizeof(char) == 1);

					valuePtr = &arg;
					valueSize = sizeof(char);
				}
				else if constexpr (std::is_same_v<T, float>)
				{
					static_assert(sizeof(float) == 4);

					void* bigEndianPtr = &bigEndianFormats[bigEndianFormatUsedSize];

					valuePtr = bigEndianPtr;
					valueSize = 4;

					float bigEndianValue = Nz::HostToNet(arg);
					std::memcpy(bigEndianPtr, &bigEndianValue, sizeof(bigEndianValue));
					bigEndianFormatUsedSize += valueSize;
				}
				else if constexpr (std::is_same_v<T, double>)
				{
					static_assert(sizeof(double) == 8);

					void* bigEndianPtr = &bigEndianFormats[bigEndianFormatUsedSize];

					valuePtr = bigEndianPtr;
					valueSize = 8;

					double bigEndianValue = Nz::HostToNet(arg);
					std::memcpy(bigEndianPtr, &bigEndianValue, sizeof(bigEndianValue));
					bigEndianFormatUsedSize += valueSize;
				}
				else if constexpr (std::is_same_v<T, Nz::Int16>)
				{
					void* bigEndianPtr = &bigEndianFormats[bigEndianFormatUsedSize];

					valuePtr = bigEndianPtr;
					valueSize = 2;

					Nz::Int16 bigEndianValue = Nz::HostToNet(arg);
					std::memcpy(bigEndianPtr, &bigEndianValue, sizeof(bigEndianValue));
					bigEndianFormatUsedSize += valueSize;
				}
				else if constexpr (std::is_same_v<T, Nz::Int32>)
				{
					void* bigEndianPtr = &bigEndianFormats[bigEndianFormatUsedSize];

					valuePtr = bigEndianPtr;
					valueSize = 4;

					Nz::Int32 bigEndianValue = Nz::HostToNet(arg);
					std::memcpy(bigEndianPtr, &bigEndianValue, sizeof(bigEndianValue));
					bigEndianFormatUsedSize += valueSize;
				}
				else if constexpr (std::is_same_v<T, Nz::Int64>)
				{
					void* bigEndianPtr = &bigEndianFormats[bigEndianFormatUsedSize];

					valuePtr = bigEndianPtr;
					valueSize = 8;

					Nz::Int64 bigEndianValue = Nz::HostToNet(arg);
					std::memcpy(bigEndianPtr, &bigEndianValue, sizeof(bigEndianValue));
					bigEndianFormatUsedSize += valueSize;
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
		Nz::StackArray<Oid> parameterIds = NazaraStackAllocationNoInit(Oid, parameterTypes.size());

		auto parameterId = parameterTypes.begin();
		for (std::size_t i = 0; i < parameterTypes.size(); ++i)
			parameterIds[i] = GetDatabaseOid(*parameterId++);

		return DatabaseResult(PQprepare(m_connection, statementName.data(), query.data(), int(parameterIds.size()), parameterIds.data()));
	}
}
