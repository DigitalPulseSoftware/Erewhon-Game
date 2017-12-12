// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ServerMatchEntities.hpp>

namespace ewn
{
	inline ServerMatchEntities::ServerMatchEntities(ServerConnection* server, Ndk::WorldHandle world) :
	m_jitterBuffer(5),
	m_jitterBufferSize(0),
	m_world(std::move(world)),
	m_correctionAccumulator(0.f),
	m_snapshotUpdateAccumulator(0.f)
	{
		m_onArenaStateSlot.Connect(server->OnArenaState, this, &ServerMatchEntities::OnArenaState);
		m_onCreateEntitySlot.Connect(server->OnCreateEntity, this, &ServerMatchEntities::OnCreateEntity);
		m_onDeleteEntitySlot.Connect(server->OnDeleteEntity, this, &ServerMatchEntities::OnDeleteEntity);

		CreateEntityTemplates();
	}

	inline ServerMatchEntities::ServerEntity& ServerMatchEntities::CreateServerEntity(std::size_t id)
	{
		if (id >= m_serverEntities.size())
			m_serverEntities.resize(id + 1);

		ServerEntity& data = m_serverEntities[id];
		assert(!data.isValid);

		data.serverId = id;
		data.isValid = true;

		return data;
	}

	inline ServerMatchEntities::ServerEntity& ServerMatchEntities::GetServerEntity(std::size_t id)
	{
		assert(IsServerEntityValid(id));
		return m_serverEntities[id];
	}

	inline bool ServerMatchEntities::IsServerEntityValid(std::size_t id) const
	{
		return id < m_serverEntities.size() && m_serverEntities[id].isValid;
	}
}
