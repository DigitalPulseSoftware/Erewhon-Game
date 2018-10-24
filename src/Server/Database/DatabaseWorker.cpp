// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Database/DatabaseWorker.hpp>
#include <Server/Database/Database.hpp>
#include <Nazara/Core/Clock.hpp>
#include <chrono>
#include <iostream>

namespace ewn
{
	constexpr Nz::UInt64 PingInterval = 10'000; //< 10s

	void DatabaseWorker::ResetIdle()
	{
		m_idle.store(false, std::memory_order_relaxed);
	}

	void DatabaseWorker::WaitForIdle()
	{
		std::unique_lock<std::mutex> lock(m_idleMutex);
		m_idleConditionVariable.wait(lock, [this] { return m_idle.load(std::memory_order_acquire); });
	}

	DatabaseResult DatabaseWorker::HandleTransactionStatement(DatabaseConnection& connection, DatabaseTransaction& transaction, const DatabaseTransaction::Statement& transactionStatement)
	{
		return std::visit([&](auto&& statement)
		{
			using T = std::decay_t<decltype(statement)>;

			DatabaseResult result;
			if constexpr (std::is_same_v<T, DatabaseTransaction::PreparedStatement>)
			{
				result = connection.ExecPreparedStatement(statement.statementName, statement.parameters);
			}
			else if constexpr (std::is_same_v<T, DatabaseTransaction::QueryStatement>)
			{
				result = connection.Exec(statement.query);
			}
			else
				static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

			if (!transactionStatement.operatorFunc)
				return result;

			return transactionStatement.operatorFunc(transaction, std::move(result));

		}, transactionStatement.statement);
	}

	void DatabaseWorker::WorkerThread()
	{
		DatabaseConnection connection = m_database.CreateConnection();
		Database::RequestQueue& queue = m_database.GetRequestQueue();

		moodycamel::ConsumerToken consumerToken(queue);

		Database::Request request;
		bool wasConnected = connection.IsConnected();

		Nz::UInt64 lastRequestTime = Nz::GetElapsedMilliseconds();

		while (m_running.load(std::memory_order_acquire))
		{
			if (!connection.IsConnected())
			{
				if (wasConnected)
					std::cerr << "Lost connection to database";
				else
					std::cerr << "Failed to connect to database: " + connection.GetLastErrorMessage();

				std::cerr << "\ntrying again in 10 seconds..." << std::endl;

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
				m_idle.store(false, std::memory_order_release);

				std::visit([&](auto&& request)
				{
					using T = std::decay_t<decltype(request)>;

					if constexpr (std::is_same_v<T, Database::QueryRequest>)
					{
						Database::QueryResult resultData;
						resultData.callback = std::move(request.callback);
						resultData.result = connection.ExecPreparedStatement(request.statement, request.parameters);

						if (!resultData.result)
							std::cerr << "[Database] statement \"" << request.statement << "\" failed: " << resultData.result.GetLastErrorMessage() << std::endl;

						m_database.SubmitResult(std::move(resultData));
					}
					else if constexpr (std::is_same_v<T, Database::TransactionRequest>)
					{
						Database::TransactionResult result;
						result.callback = std::move(request.callback);
						result.results.reserve(request.transaction.size() + 2); //< + BEGIN/COMMIT results

						DatabaseResult& beginResult = result.results.emplace_back(connection.Exec("START TRANSACTION"));
						if (beginResult)
						{
							bool failure = false;
							for (std::size_t i = 0; i < request.transaction.size(); ++i)
							{
								DatabaseResult& statementResult = result.results.emplace_back(HandleTransactionStatement(connection, request.transaction, request.transaction[i]));

								if (!statementResult)
								{
									std::cerr << "[Database] Transaction failed: " << statementResult.GetLastErrorMessage();

									failure = true;
									if (connection.IsConnected())
									{
										DatabaseResult rollbackResult = connection.Exec("ROLLBACK");
										if (!rollbackResult)
											std::cerr << "[Database] Rollback failed: " << rollbackResult.GetLastErrorMessage();
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

				lastRequestTime = Nz::GetElapsedMilliseconds();
			}
			else
			{
				m_idle.store(true, std::memory_order_release);
				m_idleConditionVariable.notify_all();

				Nz::UInt64 now = Nz::GetElapsedMilliseconds();
				if (now - lastRequestTime > PingInterval)
				{
					lastRequestTime = Nz::GetElapsedMilliseconds();

					auto pingResult = connection.ExecPreparedStatement("Ping", {});
					if (!pingResult)
						continue;
				}
			}
		}
	}
}
