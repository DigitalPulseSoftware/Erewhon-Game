// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerApplication.hpp>

namespace ewn
{
	inline void ServerApplication::DispatchWork(WorkerFunction workFunc)
	{
		m_workerQueue.enqueue(std::move(workFunc));
	}

	inline Database& ServerApplication::GetGlobalDatabase()
	{
		assert(m_globalDatabase.has_value());
		return *m_globalDatabase;
	}

	inline Arena* ServerApplication::GetArena(std::size_t arenaIndex) const
	{
		assert(arenaIndex < m_arenas.size());
		return m_arenas[arenaIndex].get();
	}

	inline std::size_t ServerApplication::GetArenaCount() const
	{
		return m_arenas.size();
	}

	inline ServerChatCommandStore& ServerApplication::GetChatCommandStore()
	{
		return m_chatCommandStore;
	}

	inline const ServerChatCommandStore& ServerApplication::GetChatCommandStore() const
	{
		return m_chatCommandStore;
	}

	inline CollisionMeshStore& ServerApplication::GetCollisionMeshStore()
	{
		return m_collisionMeshStore;
	}

	inline const CollisionMeshStore& ServerApplication::GetCollisionMeshStore() const
	{
		return m_collisionMeshStore;
	}

	inline const ServerApplication::DefaultSpaceship& ServerApplication::GetDefaultSpaceshipData() const
	{
		return m_defaultSpaceshipData;
	}

	inline ModuleStore& ServerApplication::GetModuleStore()
	{
		return m_moduleStore;
	}

	inline const ModuleStore& ServerApplication::GetModuleStore() const
	{
		return m_moduleStore;
	}

	inline std::size_t ServerApplication::GetPeerPerReactor() const
	{
		return m_peerPerReactor;
	}

	inline Player* ServerApplication::GetPlayerBySession(std::size_t sessionId)
	{
		auto it = m_sessionIdToPeer.find(sessionId);
		if (it != m_sessionIdToPeer.end())
			return m_sessions[it->second]->GetPlayer();
		else
			return nullptr;
	}

	inline const NetworkStringStore& ServerApplication::GetNetworkStringStore() const
	{
		return m_stringStore;
	}

	inline SpaceshipHullStore& ServerApplication::GetSpaceshipHullStore()
	{
		return m_spaceshipHullStore;
	}

	inline const SpaceshipHullStore& ServerApplication::GetSpaceshipHullStore() const
	{
		return m_spaceshipHullStore;
	}

	inline VisualMeshStore& ServerApplication::GetVisualMeshStore()
	{
		return m_visualMeshStore;
	}

	inline const VisualMeshStore& ServerApplication::GetVisualMeshStore() const
	{
		return m_visualMeshStore;
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
