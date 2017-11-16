// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_PLAYERCONTROLLEDCOMPONENT_HPP
#define EREWHON_SERVER_PLAYERCONTROLLEDCOMPONENT_HPP

#include <Nazara/Math/Vector3.hpp>
#include <NDK/Component.hpp>

namespace ewn
{
	class PlayerControlledComponent : public Ndk::Component<PlayerControlledComponent>
	{
		public:
			inline PlayerControlledComponent();

			inline const Nz::Vector3f& GetDirection() const;
			inline const Nz::Vector3f& GetRotation() const;

			inline void Update(const Nz::Vector3f& direction, const Nz::Vector3f& rotation);

			static Ndk::ComponentIndex componentIndex;

		private:
			Nz::Vector3f m_direction;
			Nz::Vector3f m_rotation;
	};
}

#include <Server/Components/PlayerControlledComponent.inl>

#endif // EREWHON_SERVER_PLAYERCONTROLLEDCOMPONENT_HPP
