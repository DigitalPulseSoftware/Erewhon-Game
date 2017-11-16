// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/PlayerControlledComponent.hpp>

namespace ewn
{
	inline PlayerControlledComponent::PlayerControlledComponent() :
	m_direction(Nz::Vector3f::Zero()),
	m_rotation(Nz::Vector3f::Zero())
	{
	}

	inline const Nz::Vector3f& PlayerControlledComponent::GetDirection() const
	{
		return m_direction;
	}

	inline const Nz::Vector3f& PlayerControlledComponent::GetRotation() const
	{
		return m_rotation;
	}

	inline void PlayerControlledComponent::Update(const Nz::Vector3f& direction, const Nz::Vector3f& rotation)
	{
		m_direction = direction;
		m_rotation = rotation;
	}
}
