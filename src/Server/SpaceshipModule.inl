// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/SpaceshipModule.hpp>

namespace ewn
{
	inline SpaceshipModule::SpaceshipModule(SpaceshipCore* core, const Ndk::EntityHandle& spaceship) :
	m_spaceship(spaceship),
	m_core(core),
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

	inline const Ndk::EntityHandle& SpaceshipModule::GetSpaceship()
	{
		return m_spaceship;
	}

	template<typename... Args>
	void SpaceshipModule::PushCallback(Args&&... args)
	{
		m_core->PushCallback(std::forward<Args>(args)...)
	}
}
