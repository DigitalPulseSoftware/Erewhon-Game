// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Database/DatabaseTransaction.hpp>

namespace ewn
{
	inline std::size_t DatabaseTransaction::AppendQuery(std::string query)
	{
		QueryStatement statement;
		statement.query = std::move(query);

		m_statements.emplace_back(std::move(statement));
		return m_statements.size();
	}

	inline std::size_t DatabaseTransaction::AppendPreparedStatement(std::string statementName, std::initializer_list<DatabaseValue> parameters)
	{
		return AppendPreparedStatement(std::move(statementName), &*parameters.begin(), parameters.size());
	}

	inline std::size_t DatabaseTransaction::AppendPreparedStatement(std::string statementName, std::vector<DatabaseValue> parameters)
	{
		PreparedStatement statement;
		statement.statementName = std::move(statementName);
		statement.parameters = std::move(parameters);

		m_statements.emplace_back(std::move(statement));
		return m_statements.size();
	}

	inline std::size_t DatabaseTransaction::AppendPreparedStatement(std::string statementName, const DatabaseValue* parameters, std::size_t parameterCount)
	{
		return AppendPreparedStatement(std::move(statementName), std::vector<DatabaseValue>(parameters, parameters + parameterCount));
	}

	inline std::size_t DatabaseTransaction::GetBeginResultIndex()
	{
		return 0;
	}

	inline std::size_t DatabaseTransaction::GetCommitResultIndex()
	{
		return m_statements.size();
	}

	inline DatabaseTransaction::const_iterator DatabaseTransaction::begin() const
	{
		return m_statements.begin();
	}

	inline bool DatabaseTransaction::empty() const
	{
		return m_statements.empty();
	}

	inline DatabaseTransaction::const_iterator DatabaseTransaction::end() const
	{
		return m_statements.end();
	}

	inline DatabaseTransaction::size_type DatabaseTransaction::size() const
	{
		return m_statements.size();
	}
}
