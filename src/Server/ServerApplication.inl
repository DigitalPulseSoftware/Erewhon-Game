// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerApplication.hpp>

namespace ewn
{
	inline void ServerApplication::DispatchWork(WorkerFunction workFunc)
	{
		m_workerQueue.enqueue(std::move(workFunc));
	}

	inline void ServerApplication::RegisterCallback(ServerCallback callback)
	{
		m_callbackQueue.enqueue(std::move(callback));
	}

	inline ServerApplication::WorkerQueue& ServerApplication::GetWorkerQueue()
	{
		return m_workerQueue;
	}
}
