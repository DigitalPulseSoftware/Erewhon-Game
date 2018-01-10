// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/SpaceshipModule.hpp>

namespace ewn
{
	inline SpaceshipModule::SpaceshipModule(const Ndk::EntityHandle& spaceship) :
	m_spaceship(spaceship),
	m_enabled(true)
	{
	}

	inline void SpaceshipModule::Disable()
	{
		return Enable(false);
	}

	inline void SpaceshipModule::Enable(bool enable)
	{
		m_enabled = enable;
	}

	inline bool SpaceshipModule::IsEnabled() const
	{
		return m_enabled;
	}
}
