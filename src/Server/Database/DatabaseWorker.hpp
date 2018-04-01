// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_DATABASEWORKER_HPP
#define EREWHON_SERVER_DATABASEWORKER_HPP

#include <Nazara/Prerequisites.hpp>
#include <Nazara/Core/Thread.hpp>
#include <Server/Database/DatabaseConnection.hpp>
#include <Server/Database/DatabaseTransaction.hpp>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>

namespace ewn
{
	class Database;

	class DatabaseWorker final
	{
		public:
			inline DatabaseWorker(Database& database);
			DatabaseWorker(const DatabaseWorker&) = delete;
			DatabaseWorker(DatabaseWorker&&) = delete;
			inline ~DatabaseWorker();

			void ResetIdle();

			void WaitForIdle();

			DatabaseWorker& operator=(const DatabaseWorker&) = delete;
			DatabaseWorker& operator=(DatabaseWorker&&) = delete;

		private:
			DatabaseResult HandleTransactionStatement(DatabaseConnection& connection, DatabaseTransaction& transaction, const DatabaseTransaction::Statement& transactionStatement);
			void WorkerThread();

			std::atomic_bool m_idle;
			std::atomic_bool m_running;
			std::condition_variable m_idleConditionVariable;
			std::mutex m_idleMutex;
			Nz::Thread m_thread;
			Database& m_database;
	};
}

#include <Server/Database/DatabaseWorker.inl>

#endif // EREWHON_SERVER_DATABASEWORKER_HPP
