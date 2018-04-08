// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/Components/SoundEmitterComponent.hpp>

namespace ewn
{
	inline const Nz::Vector3f& ewn::SoundEmitterComponent::GetLastPosition() const
	{
		return m_lastPosition;
	}

	inline void SoundEmitterComponent::UpdateLastPosition(const Nz::Vector3f& position)
	{
		m_lastPosition = position;
	}
}
