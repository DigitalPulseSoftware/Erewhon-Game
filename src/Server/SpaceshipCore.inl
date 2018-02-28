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
		auto SortCallbacks = [](const Callback& lhs, const Callback& rhs)
		{
			return lhs.triggerTime > rhs.triggerTime;
		};

		// Check if callback is already in the waiting queue
		auto it = m_pushedCallbacks.find(callbackName);
		if (it == m_pushedCallbacks.end())
			it = m_pushedCallbacks.emplace(callbackName, false).first;
		else if (it->second)
		{
			// If callback is already present, update its trigger time and resort callback
			for (auto callbackIt = m_callbacks.begin(); callbackIt != m_callbacks.end(); ++callbackIt)
			{
				if (callbackIt->callbackName == callbackName)
				{
					callbackIt->triggerTime = triggerTime;
					std::sort(m_callbacks.begin(), m_callbacks.end(), SortCallbacks);
					return;
				}
			}
		}

		it->second = true;

		// Insert a new callback in the queue
		Callback callback;
		callback.callbackName = std::move(callbackName);
		callback.triggerTime = triggerTime;

		auto callbackIt = std::upper_bound(m_callbacks.begin(), m_callbacks.end(), callback, SortCallbacks);

		m_callbacks.emplace(callbackIt, std::move(callback));
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

		auto it = m_pushedCallbacks.find(callback.callbackName);
		assert(it != m_pushedCallbacks.end() && it->second);
		it->second = false;

		std::cout << "Executing " << callback.callbackName << " (late by " << (now - callback.triggerTime) << "ms)" << std::endl;

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
