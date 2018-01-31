// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Database/DatabaseTypes.hpp>
#include <limits>

namespace ewn
{
	unsigned int GetDatabaseOid(DatabaseType type)
	{
		switch (type)
		{
			// From catalog/pg_types.h (Thanks to DragonJoker)
			case DatabaseType::Binary:
				return 17;

			case DatabaseType::Bool:
				return 16;

			case DatabaseType::Char:
				return 18;

			case DatabaseType::Double:
				return 701;

			case DatabaseType::Int16:
				return 21;

			case DatabaseType::Int32:
				return 23;

			case DatabaseType::Int64:
				return 20;

			case DatabaseType::Single:
				return 700;

			case DatabaseType::Text:
				return 25;

			case DatabaseType::Varchar:
				return 1043;
		}

		return std::numeric_limits<unsigned int>::max();
	}
}
