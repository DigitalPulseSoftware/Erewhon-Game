// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Database/DatabaseResult.hpp>
#include <Nazara/Core/ByteArray.hpp>
#include <Nazara/Network/Algorithm.hpp>
#include <pgsql/libpq-fe.h>
#include <array>
#include <cassert>
#include <sstream>
#include <stdexcept>

namespace ewn
{
	DatabaseResult::~DatabaseResult()
	{
		if (m_result)
			PQclear(m_result);
	}

	std::size_t DatabaseResult::GetColumnCount() const
	{
		return PQnfields(m_result);
	}

	const char* DatabaseResult::GetColumnName(std::size_t columnIndex) const
	{
		return PQfname(m_result, int(columnIndex));
	}

	std::string DatabaseResult::GetLastErrorMessage() const
	{
		return PQresultErrorMessage(m_result);
	}

	std::size_t DatabaseResult::GetRowCount() const
	{
		return PQntuples(m_result);
	}

	DatabaseValue DatabaseResult::GetValue(std::size_t columnIndex, std::size_t rowIndex) const
	{
		if (PQfformat(m_result, int(columnIndex)))
		{
			const Nz::UInt8* dataPtr = reinterpret_cast<const Nz::UInt8*>(PQgetvalue(m_result, int(rowIndex), int(columnIndex)));
			std::size_t dataSize = PQgetlength(m_result, int(rowIndex), int(columnIndex));

			Oid typeId = PQftype(m_result, int(columnIndex));
			switch (typeId)
			{
				case GetDatabaseOid(DatabaseType::Binary):
					return std::vector<Nz::UInt8>(dataPtr, dataPtr + dataSize);

				case GetDatabaseOid(DatabaseType::Bool):
					assert(dataSize == 1);
					return (*dataPtr == 1);

				case GetDatabaseOid(DatabaseType::Char):
					assert(dataSize == 1);
					return static_cast<char>(*dataPtr);

				case GetDatabaseOid(DatabaseType::Double):
				{
					static_assert(sizeof(double) == 8);

					assert(dataSize == 8);
					return Nz::NetToHost(*reinterpret_cast<const double*>(dataPtr));
				}

				case GetDatabaseOid(DatabaseType::Int16):
					assert(dataSize == 2);
					return Nz::NetToHost(*reinterpret_cast<const Nz::Int16*>(dataPtr));

				case GetDatabaseOid(DatabaseType::Date): //< Fixme
				case GetDatabaseOid(DatabaseType::Int32):
					assert(dataSize == 4);
					return Nz::NetToHost(*reinterpret_cast<const Nz::Int32*>(dataPtr));

				case GetDatabaseOid(DatabaseType::Time): //< Fixme
				case GetDatabaseOid(DatabaseType::Int64):
					assert(dataSize == 8);
					return Nz::NetToHost(*reinterpret_cast<const Nz::Int64*>(dataPtr));

				case GetDatabaseOid(DatabaseType::Single):
				{
					static_assert(sizeof(float) == 4);

					assert(dataSize == 4);
					return Nz::NetToHost(*reinterpret_cast<const float*>(dataPtr));
				}

				case GetDatabaseOid(DatabaseType::FixedVarchar):
				case GetDatabaseOid(DatabaseType::Text):
				case GetDatabaseOid(DatabaseType::Varchar):
				{
					const char* data = reinterpret_cast<const char*>(dataPtr);
					return std::string(data, data + dataSize);
				}

				default:
					throw std::runtime_error("Unhandled type " + std::to_string(typeId));
			}
		}
		else
			return PQgetvalue(m_result, int(rowIndex), int(columnIndex));
	}

	bool DatabaseResult::IsNull(std::size_t columnIndex, std::size_t rowIndex) const
	{
		return PQgetisnull(m_result, int(rowIndex), int(columnIndex));
	}

	bool DatabaseResult::IsValid() const
	{
		switch (PQresultStatus(m_result))
		{
			case PGRES_COMMAND_OK:
			case PGRES_TUPLES_OK:
			case PGRES_COPY_OUT:
			case PGRES_COPY_IN:
			case PGRES_COPY_BOTH:
			case PGRES_SINGLE_TUPLE:
				return true;

			default:
				return false;
		}
	}

	std::string DatabaseResult::ToString() const
	{
		std::ostringstream ss;

		ExecStatusType result = PQresultStatus(m_result);
		if (result == PGRES_TUPLES_OK)
		{
			std::size_t columnCount = GetColumnCount();
			std::size_t rowCount = GetRowCount();
			for (std::size_t j = 0; j < columnCount; ++j)
			{
				if (j != 0)
					ss << " | ";

				ss << GetColumnName(j);
			}
			ss << std::endl;

			for (std::size_t i = 0; i < rowCount; ++i)
			{
				for (std::size_t j = 0; j < columnCount; ++j)
				{
					if (j != 0)
						ss << " | ";

					std::visit([&](auto&& arg)
					{
						using T = std::decay_t<decltype(arg)>;

						if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, char> ||
						              std::is_same_v<T, float> || std::is_same_v<T, double> ||
						              std::is_same_v<T, Nz::Int16> || std::is_same_v<T, Nz::Int32> ||
						              std::is_same_v<T, Nz::Int64> || std::is_same_v<T, const char*> ||
						              std::is_same_v<T, std::string>)
						{
							ss << arg;
						}
						else if constexpr (std::is_same_v<T, std::vector<Nz::UInt8>>)
						{
							ss << "<binary>";
						}
						else
							static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

					}, GetValue(j, i));
				}

				ss << std::endl;
			}
		}
		else
		{
			const char* errorMessage = PQresultErrorMessage(m_result);
			ss << "Query(" << PQresStatus(result) << ')';
			if (std::strcmp(errorMessage, "") != 0)
				ss << ": " << errorMessage;
		}

		return ss.str();
	}
}
