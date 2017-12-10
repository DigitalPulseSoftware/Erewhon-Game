// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ServerMatchEntities.hpp>

namespace ewn
{
	inline ServerMatchEntities::ServerMatchEntities(ServerConnection* server, Ndk::WorldHandle world) :
	m_jitterBuffer(5),
	m_jitterBufferSize(0),
	m_world(std::move(world))
	{
		m_onArenaStateSlot.Connect(server->OnArenaState, this, &ServerMatchEntities::OnArenaState);

		CreateEntityTemplates();
	}

	template<typename T>
	inline bool ServerMatchEntities::HandleSnapshot(T&& callback)
	{
		if (m_jitterBufferSize == 0)
			return false;

		auto& snapshot = m_jitterBuffer.front();

		bool isSnapshotValid = snapshot.isValid;
		if (isSnapshotValid)
		{
			const auto& snapshotView = snapshot;

			callback(snapshotView);
			snapshot.isValid = false;
		}

		std::rotate(m_jitterBuffer.begin(), m_jitterBuffer.begin() + 1, m_jitterBuffer.end());
		m_jitterBufferSize--;

		return isSnapshotValid;
	}

	inline ServerMatchEntities::ServerEntity& ServerMatchEntities::CreateServerEntity(std::size_t id)
	{
		if (id >= m_serverEntities.size())
			m_serverEntities.resize(id + 1);

		ServerEntity& data = m_serverEntities[id];
		assert(!data.isValid);

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
