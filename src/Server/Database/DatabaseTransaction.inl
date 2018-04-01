// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Database/DatabaseTransaction.hpp>

namespace ewn
{
	inline std::size_t DatabaseTransaction::AppendQuery(std::string query, TransactionOperator transactionOperator)
	{
		QueryStatement queryStatement;
		queryStatement.query = std::move(query);

		Statement statement;
		statement.operatorFunc = std::move(transactionOperator);
		statement.statement = std::move(queryStatement);

		m_statements.emplace_back(std::move(statement));
		return m_statements.size();
	}

	inline std::size_t DatabaseTransaction::AppendPreparedStatement(std::string statementName, std::initializer_list<DatabaseValue> parameters, TransactionOperator transactionOperator)
	{
		return AppendPreparedStatement(std::move(statementName), &*parameters.begin(), parameters.size(), std::move(transactionOperator));
	}

	inline std::size_t DatabaseTransaction::AppendPreparedStatement(std::string statementName, std::vector<DatabaseValue> parameters, TransactionOperator transactionOperator)
	{
		PreparedStatement preparedStatement;
		preparedStatement.parameters = std::move(parameters);
		preparedStatement.statementName = std::move(statementName);

		Statement statement;
		statement.operatorFunc = std::move(transactionOperator);
		statement.statement = std::move(preparedStatement);

		m_statements.emplace_back(std::move(statement));
		return m_statements.size();
	}

	inline std::size_t DatabaseTransaction::AppendPreparedStatement(std::string statementName, const DatabaseValue* parameters, std::size_t parameterCount, TransactionOperator transactionOperator)
	{
		return AppendPreparedStatement(std::move(statementName), std::vector<DatabaseValue>(parameters, parameters + parameterCount), std::move(transactionOperator));
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

	inline DatabaseTransaction::Statement& DatabaseTransaction::operator[](std::size_t i)
	{
		return m_statements[i];
	}

	inline const DatabaseTransaction::Statement& DatabaseTransaction::operator[](std::size_t i) const
	{
		return m_statements[i];
	}
}
