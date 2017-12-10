// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/ProjectileComponent.hpp>

namespace ewn
{
	inline ProjectileComponent::ProjectileComponent(Nz::UInt16 damageValue) :
	m_damageValue(damageValue)
	{
	}

	inline Nz::UInt16 ProjectileComponent::GetDamageValue() const
	{
		return m_damageValue;
	}

	inline bool ProjectileComponent::HasBeenHit(Ndk::Entity* entity) const
	{
		return m_hitEntities.Has(entity);
	}

	inline void ProjectileComponent::MarkAsHit(Ndk::Entity* entity)
	{
		m_hitEntities.Insert(entity);
	}
}
