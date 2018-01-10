// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_PROJECTILECOMPONENT_HPP
#define EREWHON_SERVER_PROJECTILECOMPONENT_HPP

#include <NDK/Component.hpp>
#include <NDK/EntityList.hpp>

namespace ewn
{
	class ProjectileComponent : public Ndk::Component<ProjectileComponent>
	{
		public:
			inline ProjectileComponent(Nz::UInt16 damageValue);

			inline Nz::UInt16 GetDamageValue() const;

			inline bool HasBeenHit(Ndk::Entity* entity) const;

			inline void MarkAsHit(Ndk::Entity* entity);

			static Ndk::ComponentIndex componentIndex;

		private:
			Ndk::EntityList m_hitEntities;
			Nz::UInt16 m_damageValue;
	};
}

#include <Server/Components/ProjectileComponent.inl>

#endif // EREWHON_SERVER_PROJECTILECOMPONENT_HPP
