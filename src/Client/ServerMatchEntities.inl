// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ServerMatchEntities.hpp>

namespace ewn
{
	inline void ewn::ServerMatchEntities::EnableSnapshotHandling(bool enable)
	{
		m_stateHandlingEnabled = enable;
	}

	inline ServerMatchEntities::ServerEntity& ServerMatchEntities::CreateServerEntity(Nz::UInt32 id)
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

	inline bool ServerMatchEntities::IsSnapshotHandlingEnabled() const
	{
		return m_stateHandlingEnabled;
	}

	inline bool ServerMatchEntities::IsServerEntityValid(std::size_t id) const
	{
		return id < m_serverEntities.size() && m_serverEntities[id].isValid;
	}
}
