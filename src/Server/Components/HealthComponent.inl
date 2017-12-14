// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/HealthComponent.hpp>

namespace ewn
{
	inline HealthComponent::HealthComponent(Nz::UInt16 maxHealth) :
	m_currentHealth(maxHealth),
	m_maxHealth(maxHealth)
	{
	}

	inline HealthComponent::HealthComponent(const HealthComponent& health) :
	m_currentHealth(health.m_currentHealth),
	m_maxHealth(health.m_maxHealth)
	{
	}

	inline void HealthComponent::Damage(Nz::UInt16 damage, const Ndk::EntityHandle& attacker)
	{
		OnHealthChange(this);

		if (damage >= m_currentHealth)
		{
			m_currentHealth = 0;
			OnDeath(this, attacker);
		}
		else
			m_currentHealth -= damage;
	}

	inline Nz::UInt16 HealthComponent::GetHealth() const
	{
		return m_currentHealth;
	}

	inline float HealthComponent::GetHealthPct() const
	{
		return 100.f * m_currentHealth / m_maxHealth;
	}

	inline Nz::UInt16 HealthComponent::GetMaxHealth() const
	{
		return m_maxHealth;
	}

	inline void HealthComponent::Heal(Nz::UInt16 heal)
	{
		Nz::UInt16 newHealth = m_currentHealth + heal;
		if (newHealth < m_currentHealth)
			// Overflow, just set to max value
			m_currentHealth = m_maxHealth;
		else
			m_currentHealth = newHealth;

		OnHealthChange(this);
	}
}
