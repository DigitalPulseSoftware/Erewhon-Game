// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Database/DatabaseWorker.hpp>
#include <Server/Database/Database.hpp>
#include <chrono>
#include <iostream>

namespace ewn
{
	DatabaseResult DatabaseWorker::HandleTransactionStatement(DatabaseConnection& connection, const DatabaseTransaction::Statement& transactionStatement)
	{
		return std::visit([&](auto&& statement)
		{
			using T = std::decay_t<decltype(statement)>;

			if constexpr (std::is_same_v<T, DatabaseTransaction::PreparedStatement>)
			{
				return connection.ExecPreparedStatement(statement.statementName, statement.parameters);
			}
			else if constexpr (std::is_same_v<T, DatabaseTransaction::QueryStatement>)
			{
				return connection.Exec(statement.query);
			}
			else
				static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

		}, transactionStatement);
	}

	void DatabaseWorker::WorkerThread()
	{
		DatabaseConnection connection = m_database.CreateConnection();
		Database::RequestQueue& queue = m_database.GetRequestQueue();

		moodycamel::ConsumerToken consumerToken(queue);

		Database::Request request;
		bool wasConnected = connection.IsConnected();
		while (m_running.load(std::memory_order_acquire))
		{
			if (!connection.IsConnected())
			{
				std::cerr << ((wasConnected) ? "Lost connection to database" : "Failed to connect to database") << ", trying again in 10 seconds..." << std::endl;
				wasConnected = false;

				Nz::Thread::Sleep(10'000);

				//TODO: Make use of PQreset? (Beware of prepared statements)
				connection = m_database.CreateConnection();
				continue;
			}
			else if (!wasConnected)
			{
				std::cout << "Connection retrieved" << std::endl;
				wasConnected = true;
			}

			if (queue.wait_dequeue_timed(consumerToken, request, std::chrono::milliseconds(100)))
			{
				std::visit([&](auto&& request)
				{
					using T = std::decay_t<decltype(request)>;

					if constexpr (std::is_same_v<T, Database::QueryRequest>)
					{
						Database::QueryResult result;
						result.callback = std::move(request.callback);
						result.result = connection.ExecPreparedStatement(request.statement, request.parameters);

						m_database.SubmitResult(std::move(result));
					}
					else if constexpr (std::is_same_v<T, Database::TransactionRequest>)
					{
						Database::TransactionResult result;
						result.callback = std::move(request.callback);
						result.results.reserve(request.transaction.size() + 2); //< + BEGIN/COMMIT results
						result.transactionSucceeded = false;

						DatabaseResult& beginResult = result.results.emplace_back(connection.Exec("START TRANSACTION"));
						if (beginResult)
						{
							bool failure = false;
							for (const auto& transactionStatement : request.transaction)
							{
								DatabaseResult& statementResult = result.results.emplace_back(HandleTransactionStatement(connection, transactionStatement));

								if (!statementResult)
								{
									failure = true;
									if (connection.IsConnected())
									{
										DatabaseResult& rollbackResult = connection.Exec("ROLLBACK");
										if (!rollbackResult)
											std::cerr << "Rollback failed: " << rollbackResult.GetLastErrorMessage();
									}
									break;
								}
							}

							if (!failure)
							{
								DatabaseResult& commitResult = result.results.emplace_back(connection.Exec("COMMIT"));
								if (commitResult)
									result.transactionSucceeded = true;
							}
						}

						m_database.SubmitResult(std::move(result));
					}
					else
						static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

				}, request);
			}
		}
	}
}
