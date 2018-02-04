// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Database/DatabaseWorker.hpp>
#include <Nazara/Core/Thread.hpp>
#include <Server/Database/Database.hpp>
#include <chrono>
#include <iostream>

namespace ewn
{
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
				Database::Result result;
				result.callback = std::move(request.callback);
				result.result = connection.ExecPreparedStatement(request.statement, request.parameters);

				m_database.SubmitResult(std::move(result));
			}
		}
	}
}
