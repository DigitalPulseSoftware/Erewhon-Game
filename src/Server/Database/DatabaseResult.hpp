// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_DATABASERESULT_HPP
#define EREWHON_SERVER_DATABASERESULT_HPP

#include <Nazara/Prerequisites.hpp>
#include <Nazara/Core/MovablePtr.hpp>
#include <Server/Database/DatabaseTypes.hpp>
#include <string>

typedef struct pg_result PGresult;

namespace ewn
{
	class DatabaseResult
	{
		public:
			inline explicit DatabaseResult(PGresult* result = nullptr);
			DatabaseResult(const DatabaseResult&) = delete;
			DatabaseResult(DatabaseResult&&) noexcept = default;
			~DatabaseResult();

			std::size_t GetAffectedRowCount() const;
			std::size_t GetColumnCount() const;
			const char* GetColumnName(std::size_t columnIndex) const;
			std::string GetLastErrorMessage() const;
			std::size_t GetRowCount() const;
			DatabaseValue GetValue(std::size_t columnIndex, std::size_t rowIndex) const;

			bool IsNull(std::size_t columnIndex, std::size_t rowIndex) const;
			bool IsValid() const;

			std::string ToString() const;

			inline explicit operator bool();

			DatabaseResult& operator=(const DatabaseResult&) = delete;
			DatabaseResult& operator=(DatabaseResult&&) noexcept = default;

		private:
			Nz::MovablePtr<PGresult> m_result;
	};
}

#include <Server/Database/DatabaseResult.inl>

#endif // EREWHON_SERVER_DATABASERESULT_HPP
