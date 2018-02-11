// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/GameWorker.hpp>
#include <Server/ServerApplication.hpp>
#include <chrono>
#include <iostream>

namespace ewn
{
	void GameWorker::WorkerThread()
	{
		ServerApplication::WorkerQueue& queue = m_app->GetWorkerQueue();

		moodycamel::ConsumerToken consumerToken(queue);

		ServerApplication::WorkerFunction job;
		while (m_running.load(std::memory_order_acquire))
		{
			if (queue.wait_dequeue_timed(consumerToken, job, std::chrono::milliseconds(100)))
				job();
		}
	}
}
