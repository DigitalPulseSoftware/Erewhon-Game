// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_GAMEWORKER_HPP
#define EREWHON_SERVER_GAMEWORKER_HPP

#include <Nazara/Prerequisites.hpp>
#include <Nazara/Core/Thread.hpp>
#include <atomic>

namespace ewn
{
	class ServerApplication;

	class GameWorker final
	{
		public:
			inline GameWorker(ServerApplication* app);
			GameWorker(const GameWorker&) = delete;
			GameWorker(GameWorker&&) = delete;
			inline ~GameWorker();

			GameWorker& operator=(const GameWorker&) = delete;
			GameWorker& operator=(GameWorker&&) = delete;

		private:
			void WorkerThread();

			std::atomic_bool m_running;
			Nz::Thread m_thread;
			ServerApplication* m_app;
	};
}

#include <Server/GameWorker.inl>

#endif // EREWHON_SERVER_GAMEWORKER_HPP
