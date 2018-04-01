// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_HEALTHCOMPONENT_HPP
#define EREWHON_SERVER_HEALTHCOMPONENT_HPP

#include <Nazara/Core/Signal.hpp>
#include <NDK/Component.hpp>

namespace ewn
{
	class HealthComponent : public Ndk::Component<HealthComponent>
	{
		public:
			inline HealthComponent(Nz::UInt16 maxHealth);
			inline HealthComponent(const HealthComponent&);

			inline void Damage(Nz::UInt16 damage, const Ndk::EntityHandle& attacker);

			inline Nz::UInt16 GetHealth() const;
			inline float GetHealthPct() const;
			inline Nz::UInt16 GetMaxHealth() const;

			inline void Heal(Nz::UInt16 heal);

			static Ndk::ComponentIndex componentIndex;

			NazaraSignal(OnDeath, HealthComponent* /*emitter*/, const Ndk::EntityHandle& /*attacker*/);
			NazaraSignal(OnHealthChange, HealthComponent* /*emitter*/);

		private:
			Nz::UInt16 m_currentHealth;
			Nz::UInt16 m_maxHealth;
	};
}

#include <Server/Components/HealthComponent.inl>

#endif // EREWHON_SERVER_HEALTHCOMPONENT_HPP
