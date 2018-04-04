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

	inline CollisionMeshStore& ServerApplication::GetCollisionMeshStore()
	{
		return m_collisionMeshStore;
	}

	inline const CollisionMeshStore& ServerApplication::GetCollisionMeshStore() const
	{
		return m_collisionMeshStore;
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

	inline void ServerApplication::RegisterCallback(ServerCallback callback)
	{
		m_callbackQueue.enqueue(std::move(callback));
	}

	inline ServerApplication::WorkerQueue& ServerApplication::GetWorkerQueue()
	{
		return m_workerQueue;
	}
}
