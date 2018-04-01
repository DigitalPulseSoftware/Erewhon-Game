// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/GameWorker.hpp>

namespace ewn
{
	inline GameWorker::GameWorker(ServerApplication* app) :
	m_running(true),
	m_app(app)
	{
		m_thread = Nz::Thread(&GameWorker::WorkerThread, this);
		m_thread.SetName("GameWorker");
	}

	inline GameWorker::~GameWorker()
	{
		// TODO: Wake thread up
		m_running.store(false, std::memory_order_release);
		m_thread.Join();
	}
}
