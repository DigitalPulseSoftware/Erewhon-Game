// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/RadarComponent.hpp>

namespace ewn
{
	inline RadarComponent::RadarComponent()
	{
	}

	inline void RadarComponent::UnwatchEntity(Ndk::Entity* entity)
	{
		m_watchedTargets.erase(entity->GetId());
	}

	inline void RadarComponent::WatchEntity(Ndk::Entity* entity, OnDestructionCallback destructionCallback, OnLeaveCallback leaveCallback)
	{
		WatchedTarget targetData;
		targetData.destructionCallback = std::move(destructionCallback);
		targetData.rangeLeaveCallback = std::move(leaveCallback);
		targetData.target = entity;
		targetData.onEntityDestroyed.Connect(entity->OnEntityDestruction, this, &RadarComponent::OnWatchedEntityDestroyed);

		m_watchedTargets.emplace(entity->GetId(), std::move(targetData));
	}
}
