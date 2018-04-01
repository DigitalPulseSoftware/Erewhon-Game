// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Database/DatabaseTypes.hpp>
#include <Shared/Utils.hpp>
#include <limits>

namespace ewn
{
	constexpr unsigned int GetDatabaseOid(DatabaseType type)
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

			case DatabaseType::Date:
				return 1082;

			case DatabaseType::Double:
				return 701;

			case DatabaseType::FixedVarchar:
				return 1042;

			case DatabaseType::Int16:
				return 21;

			case DatabaseType::Int32:
				return 23;

			case DatabaseType::Int64:
				return 20;

			case DatabaseType::Json:
				return 114;

			/*case DatabaseType::JsonBinary:
				return 3802;*/

			case DatabaseType::Single:
				return 700;

			case DatabaseType::Text:
				return 25;

			case DatabaseType::Time:
				return 1083;

			case DatabaseType::Varchar:
				return 1043;
		}

		return std::numeric_limits<unsigned int>::max();
	}

	template<typename T>
	constexpr DatabaseType GetDatabaseType()
	{
		static_assert(AlwaysFalse<T>::value, "Unknown");
	}

	template<>
	constexpr DatabaseType GetDatabaseType<bool>()
	{
		return DatabaseType::Bool;
	}

	template<>
	constexpr DatabaseType GetDatabaseType<char>()
	{
		return DatabaseType::Char;
	}

	template<>
	constexpr DatabaseType GetDatabaseType<float>()
	{
		return DatabaseType::Single;
	}

	template<>
	constexpr DatabaseType GetDatabaseType<double>()
	{
		return DatabaseType::Double;
	}

	template<>
	constexpr DatabaseType GetDatabaseType<Nz::Int16>()
	{
		return DatabaseType::Int16;
	}

	template<>
	constexpr DatabaseType GetDatabaseType<Nz::Int32>()
	{
		return DatabaseType::Int32;
	}

	template<>
	constexpr DatabaseType GetDatabaseType<Nz::Int64>()
	{
		return DatabaseType::Int64;
	}

	template<>
	constexpr DatabaseType GetDatabaseType<nlohmann::json>()
	{
		return DatabaseType::Json;
	}

	template<>
	constexpr DatabaseType GetDatabaseType<const char*>()
	{
		return DatabaseType::Varchar;
	}

	template<>
	constexpr DatabaseType GetDatabaseType<std::string>()
	{
		return DatabaseType::Varchar;
	}

	template<>
	constexpr DatabaseType GetDatabaseType<std::vector<Nz::UInt8>>()
	{
		return DatabaseType::Varchar;
	}
}
