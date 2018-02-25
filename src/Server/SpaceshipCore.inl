// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/SpaceshipCore.hpp>
#include <Server/ServerApplication.hpp>

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
		PushCallback(ServerApplication::GetAppTime(), std::move(callbackName));
	}

	inline void SpaceshipCore::PushCallback(Nz::UInt64 triggerTime, std::string callbackName)
	{
		Callback callback;
		callback.callbackName = std::move(callbackName);
		callback.triggerTime = triggerTime;

		auto it = std::upper_bound(m_callbacks.begin(), m_callbacks.end(), callback, [](const Callback& lhs, const Callback& rhs)
		{
			return lhs.triggerTime > rhs.triggerTime;
		});

		m_callbacks.emplace(it, std::move(callback));
	}

	inline std::optional<std::string> SpaceshipCore::PopCallback()
	{
		if (m_callbacks.empty())
			return {};

		Nz::UInt64 now = ServerApplication::GetAppTime();
		if (m_callbacks.back().triggerTime >= now)
			return {};

		Callback callback = std::move(m_callbacks.back());
		m_callbacks.pop_back();

		return callback.callbackName;
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
