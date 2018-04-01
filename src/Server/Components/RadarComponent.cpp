// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/RadarComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>

namespace ewn
{
	RadarComponent::RadarComponent(const RadarComponent& radar) :
	Component(radar)
	{
		for (const auto& [targetId, targetData] : m_lockedTargets)
		{
			NazaraUnused(targetId);

			LockEntity(targetData.target, targetData.destructionCallback, targetData.rangeLeaveCallback);
		}
	}

	void RadarComponent::ClearLockedTargets()
	{
		m_lockedTargets.clear();
	}

	void RadarComponent::CheckTargetRange(const Nz::Vector3f& position, float maxRange)
	{
		float maxRangeSq = maxRange * maxRange;
		for (auto it = m_lockedTargets.begin(); it != m_lockedTargets.end();)
		{
			LockedTarget& targetData = it->second;

			Ndk::NodeComponent& targetNode = targetData.target->GetComponent<Ndk::NodeComponent>();
			if (targetNode.GetPosition().SquaredDistance(position) > maxRangeSq)
			{
				targetData.rangeLeaveCallback(targetData.target);
				it = m_lockedTargets.erase(it);
			}
			else
				++it;
		}
	}

	void RadarComponent::OnWatchedEntityDestroyed(Ndk::Entity* entity)
	{
		auto it = m_lockedTargets.find(entity->GetId());
		assert(it != m_lockedTargets.end());

		it->second.destructionCallback(entity);
		m_lockedTargets.erase(it);
	}

	Ndk::ComponentIndex RadarComponent::componentIndex;
}
