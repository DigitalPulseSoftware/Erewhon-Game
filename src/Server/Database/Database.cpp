// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Database/Database.hpp>

namespace ewn
{
	DatabaseConnection Database::CreateConnection()
	{
		DatabaseConnection connection = DatabaseConnection(m_dbHostname, std::to_string(m_dbPort), m_dbUsername, m_dbPassword, m_dbName);
		if (connection.IsConnected())
			PrepareStatements(connection);

		return connection;
	}

	void Database::ExecuteQuery(std::string statement, std::vector<DatabaseValue> parameters, Callback callback)
	{
		Request newRequest;
		newRequest.callback = std::move(callback);
		newRequest.parameters = std::move(parameters);
		newRequest.statement = std::move(statement);

		m_requestQueue.enqueue(newRequest);
	}

	void Database::Poll()
	{
		Result result;
		while (m_resultQueue.try_dequeue(result))
			result.callback(result.result);
	}

	void Database::SpawnWorkers(std::size_t workerCount)
	{
		for (std::size_t i = 0; i < workerCount; ++i)
			m_workers.emplace_back(std::make_unique<DatabaseWorker>(*this));
	}

	void Database::PrepareStatement(DatabaseConnection& connection, const std::string& statementName, const std::string& query, std::initializer_list<DatabaseType> parameterTypes)
	{
		DatabaseResult result = connection.PrepareStatement(statementName, query, parameterTypes);
		if (!result.IsValid())
			throw std::runtime_error("Failed to prepare statement \"" + statementName + "\": " + result.GetLastErrorMessage());
	}
}
