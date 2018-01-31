// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Database/DatabaseTypes.hpp>

namespace ewn
{
	template<typename T>
	constexpr DatabaseType GetDatabaseType()
	{
		static_assert(false, "Unknown");
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
