// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_DATABASEWORKER_HPP
#define EREWHON_SERVER_DATABASEWORKER_HPP

#include <Nazara/Prerequisites.hpp>
#include <Nazara/Core/Thread.hpp>
#include <Server/Database/DatabaseConnection.hpp>
#include <atomic>
#include <string>

namespace ewn
{
	class Database;

	class DatabaseWorker final
	{
		public:
			inline DatabaseWorker(Database& database, DatabaseConnection connection);
			DatabaseWorker(const DatabaseWorker&) = delete;
			DatabaseWorker(DatabaseWorker&&) = delete;
			~DatabaseWorker();

			DatabaseWorker& operator=(const DatabaseWorker&) = delete;
			DatabaseWorker& operator=(DatabaseWorker&&) = delete;

		private:
			void WorkerThread();

			std::atomic_bool m_running;
			Nz::Thread m_thread;
			Database& m_database;
			DatabaseConnection m_connection;
	};
}

#include <Server/Database/DatabaseWorker.inl>

#endif // EREWHON_SERVER_DATABASEWORKER_HPP
