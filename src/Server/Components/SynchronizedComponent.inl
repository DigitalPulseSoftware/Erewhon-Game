// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/SynchronizedComponent.hpp>
#include <limits>

namespace ewn
{
	inline SynchronizedComponent::SynchronizedComponent(std::size_t prefabId, std::string type, std::string nameTemp, bool movable, Nz::UInt16 networkPriority) :
	m_prefabId(prefabId),
	m_name(std::move(nameTemp)),
	m_type(std::move(type)),
	m_priority(networkPriority),
	m_priorityAccumulator(networkPriority),
	m_movable(movable)
	{
	}

	inline void SynchronizedComponent::AccumulatePriority()
	{
		Nz::UInt16 newPriority = m_priorityAccumulator + m_priority;
		if (newPriority < m_priorityAccumulator) // Overflow protection
			m_priorityAccumulator = std::numeric_limits<decltype(m_priorityAccumulator)>::max();
		else
			m_priorityAccumulator = newPriority;
	}

	inline const std::string& SynchronizedComponent::GetName() const
	{
		return m_name;
	}

	inline std::size_t SynchronizedComponent::GetPrefabId() const
	{
		return m_prefabId;
	}

	inline Nz::UInt16 SynchronizedComponent::GetPriority() const
	{
		return m_priority;
	}

	inline Nz::UInt16 SynchronizedComponent::GetPriorityAccumulator() const
	{
		return m_priorityAccumulator;
	}

	inline const std::string& SynchronizedComponent::GetType() const
	{
		return m_type;
	}

	inline bool SynchronizedComponent::IsMovable() const
	{
		return m_movable;
	}

	inline void SynchronizedComponent::ResetPriorityAccumulator()
	{
		m_priorityAccumulator = 0;
	}
}
