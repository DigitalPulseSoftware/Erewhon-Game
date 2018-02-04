// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Database/DatabaseWorker.hpp>

namespace ewn
{
	inline DatabaseWorker::DatabaseWorker(Database& database) :
	m_running(true),
	m_database(database)
	{
		m_thread = Nz::Thread(&DatabaseWorker::WorkerThread, this);
		m_thread.SetName("DatabaseWorker");
	}

	inline DatabaseWorker::~DatabaseWorker()
	{
		// TODO: Wake thread up
		m_running.store(false, std::memory_order_release);
		m_thread.Join();
	}
}
