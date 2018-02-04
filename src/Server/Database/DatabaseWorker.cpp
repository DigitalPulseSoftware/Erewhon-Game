// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Database/DatabaseWorker.hpp>
#include <Server/Database/Database.hpp>
#include <chrono>

namespace ewn
{
	void DatabaseWorker::WorkerThread()
	{
		Database::RequestQueue& queue = m_database.GetRequestQueue();

		moodycamel::ConsumerToken consumerToken(queue);

		Database::Request request;
		while (m_running.load(std::memory_order_acquire))
		{
			if (queue.wait_dequeue_timed(consumerToken, request, std::chrono::milliseconds(100)))
			{
				Database::Result result;
				result.callback = std::move(request.callback);
				result.result = m_connection.ExecPreparedStatement(request.statement, request.parameters);

				m_database.SubmitResult(std::move(result));
			}
		}
	}
}
