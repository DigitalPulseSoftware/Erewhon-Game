// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Database/DatabaseResult.hpp>
#include <pgsql/libpq-fe.h>
#include <array>
#include <cassert>
#include <sstream>

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

	std::size_t DatabaseResult::GetRowCount() const
	{
		return PQntuples(m_result);
	}

	const char* DatabaseResult::GetValue(std::size_t columnIndex, std::size_t rowIndex) const
	{
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

		if (PQresultStatus(m_result) == PGRES_TUPLES_OK)
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

					ss << GetValue(j, i);
				}

				ss << std::endl;
			}
		}
		else
		{
			const char* errorMessage = PQresultErrorMessage(m_result);
			ss << "Query(" << PQresStatus(PQresultStatus(m_result)) << ')';
			if (std::strcmp(errorMessage, "") != 0)
				ss << ": " << errorMessage;
		}

		return ss.str();
	}
}
