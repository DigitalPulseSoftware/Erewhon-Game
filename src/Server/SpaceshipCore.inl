// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/SpaceshipCore.hpp>

namespace ewn
{
	inline SpaceshipCore::SpaceshipCore(const Ndk::EntityHandle& spaceship) :
	m_spaceship(spaceship)
	{
	}

	inline void SpaceshipCore::AddModule(std::shared_ptr<SpaceshipModule> modulePtr)
	{
		m_modules.emplace_back(std::move(modulePtr));
	}

	inline void SpaceshipCore::PushCallback(std::string callbackName)
	{
		m_callbacks.emplace(m_callbacks.begin(), std::move(callbackName));
	}

	inline std::optional<std::string> SpaceshipCore::PopCallback()
	{
		if (m_callbacks.empty())
			return {};

		std::string callbackName = std::move(m_callbacks.back());
		m_callbacks.pop_back();

		return callbackName;
	}
}

namespace Nz
{
	inline int LuaImplReplyVal(const LuaState& state, ewn::SpaceshipCore* ptr, TypeTag<ewn::SpaceshipCore*>)
	{
		state.PushInstance<ewn::SpaceshipCoreHandle>("Core", ptr);
		return 1;
	}
}
