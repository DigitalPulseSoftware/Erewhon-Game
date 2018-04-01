// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_PLAYERCONTROLLEDCOMPONENT_HPP
#define EREWHON_SERVER_PLAYERCONTROLLEDCOMPONENT_HPP

#include <Nazara/Math/Vector3.hpp>
#include <NDK/Component.hpp>
#include <Server/Player.hpp>

namespace ewn
{
	class PlayerControlledComponent : public Ndk::Component<PlayerControlledComponent>
	{
		public:
			inline PlayerControlledComponent(Player* owner);

			inline Player* GetOwner() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			PlayerHandle m_owner;
	};
}

#include <Server/Components/PlayerControlledComponent.inl>

#endif // EREWHON_SERVER_PLAYERCONTROLLEDCOMPONENT_HPP
