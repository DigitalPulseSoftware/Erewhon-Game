// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/OwnerComponent.hpp>

namespace ewn
{
	inline OwnerComponent::OwnerComponent(Player* owner) :
	m_owner(owner)
	{
	}

	inline Player* OwnerComponent::GetOwner() const
	{
		return m_owner;
	}
}
