// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/RadarComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>

namespace ewn
{
	RadarComponent::RadarComponent(const RadarComponent& radar) :
	Component(radar)
	{
		for (const auto&[targetId, targetData] : m_watchedTargets)
		{
			NazaraUnused(targetId);

			WatchEntity(targetData.target, targetData.destructionCallback, targetData.rangeLeaveCallback);
		}
	}

	void RadarComponent::ClearWatchedTargets()
	{
		m_watchedTargets.clear();
	}

	void RadarComponent::CheckTargetRange(const Nz::Vector3f& position, float maxRange)
	{
		float maxRangeSq = maxRange * maxRange;
		for (auto it = m_watchedTargets.begin(); it != m_watchedTargets.end(); ++it)
		{
			WatchedTarget& targetData = it->second;

			Ndk::NodeComponent& targetNode = targetData.target->GetComponent<Ndk::NodeComponent>();
			if (targetNode.GetPosition().SquaredDistance(position) > maxRangeSq)
			{
				targetData.rangeLeaveCallback(targetData.target);
				it = m_watchedTargets.erase(it);
			}
			else
				++it;
		}
	}

	void RadarComponent::OnWatchedEntityDestroyed(Ndk::Entity* entity)
	{
		UnwatchEntity(entity);
	}

	Ndk::ComponentIndex RadarComponent::componentIndex;
}
