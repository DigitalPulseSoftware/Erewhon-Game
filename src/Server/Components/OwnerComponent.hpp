// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_OWNERCOMPONENT_HPP
#define EREWHON_SERVER_OWNERCOMPONENT_HPP

#include <NDK/Component.hpp>
#include <Server/Player.hpp>

namespace ewn
{
	class OwnerComponent : public Ndk::Component<OwnerComponent>
	{
		public:
			inline OwnerComponent(Player* owner);

			inline Player* GetOwner() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			PlayerHandle m_owner;
	};
}

#include <Server/Components/OwnerComponent.inl>

#endif // EREWHON_SERVER_OWNERCOMPONENT_HPP
