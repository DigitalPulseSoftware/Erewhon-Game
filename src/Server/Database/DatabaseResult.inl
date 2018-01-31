// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Database/DatabaseConnection.hpp>

namespace ewn
{
	inline DatabaseResult::DatabaseResult(PGresult* result) :
	m_result(result)
	{
	}

	inline DatabaseResult::operator bool()
	{
		return IsValid();
	}
}
