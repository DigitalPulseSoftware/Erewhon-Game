// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/RadarComponent.hpp>

namespace ewn
{
	inline std::size_t ewn::RadarComponent::GetLockedEntityCount() const
	{
		return m_lockedTargets.size();
	}

	inline bool RadarComponent::IsEntityLocked(Ndk::EntityId entityId)
	{
		return m_lockedTargets.find(entityId) != m_lockedTargets.end();
	}

	inline void RadarComponent::LockEntity(Ndk::Entity* entity, OnDestructionCallback destructionCallback, OnLeaveCallback leaveCallback)
	{
		LockedTarget targetData;
		targetData.destructionCallback = std::move(destructionCallback);
		targetData.rangeLeaveCallback = std::move(leaveCallback);
		targetData.target = entity;
		targetData.onEntityDestroyed.Connect(entity->OnEntityDestruction, this, &RadarComponent::OnWatchedEntityDestroyed);

		m_lockedTargets.emplace(entity->GetId(), std::move(targetData));
	}

	inline void RadarComponent::UnlockEntity(Ndk::Entity* entity)
	{
		m_lockedTargets.erase(entity->GetId());
	}
}
