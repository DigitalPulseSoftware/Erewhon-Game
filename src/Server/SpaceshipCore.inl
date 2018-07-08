// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/SpaceshipCore.hpp>
#include <Server/ServerApplication.hpp>
#include <cassert>

namespace ewn
{
	inline SpaceshipCore::SpaceshipCore(ServerApplication* app, const Ndk::EntityHandle& spaceship) :
	m_spaceship(spaceship),
	m_app(app)
	{
	}

	template<typename T>
	T* SpaceshipCore::GetModule(ModuleType type)
	{
		std::size_t typeIndex = static_cast<std::size_t>(type);
		if (typeIndex >= m_modules.size())
			return nullptr;

		SpaceshipModule* spaceshipModule = m_modules[typeIndex].get();
		assert(dynamic_cast<T*>(spaceshipModule) != nullptr && "Incompatible types");

		return static_cast<T*>(spaceshipModule);
	}

	ServerApplication* SpaceshipCore::GetApp()
	{
		return m_app;
	}

	inline void SpaceshipCore::PushCallback(std::string callbackName, CallbackArgFunction argFunc, bool unique)
	{
		PushCallback(m_app->GetAppTime(), std::move(callbackName), std::move(argFunc), unique);
	}

	inline void SpaceshipCore::PushCallback(Nz::UInt64 triggerTime, std::string callbackName, CallbackArgFunction argFunc, bool unique)
	{
		auto SortCallbacks = [](const Callback& lhs, const Callback& rhs)
		{
			return lhs.triggerTime > rhs.triggerTime;
		};

		// Check if callback is already in the waiting queue
		if (unique)
		{
			auto it = m_pushedCallbacks.find(callbackName);
			if (it == m_pushedCallbacks.end())
				it = m_pushedCallbacks.emplace(callbackName, true).first;
			else if (it->second)
			{
				// If callback is already present, update its trigger time and resort callback
				for (auto callbackIt = m_callbacks.begin(); callbackIt != m_callbacks.end(); ++callbackIt)
				{
					if (callbackIt->callbackName == callbackName)
					{
						callbackIt->argFunc = std::move(argFunc);
						callbackIt->triggerTime = triggerTime;
						std::sort(m_callbacks.begin(), m_callbacks.end(), SortCallbacks);
						return;
					}
				}
			}
			else
				it->second = true;
		}

		// Insert a new callback in the queue
		Callback callback;
		callback.argFunc = std::move(argFunc);
		callback.callbackName = std::move(callbackName);
		callback.triggerTime = triggerTime;

		auto callbackIt = std::upper_bound(m_callbacks.begin(), m_callbacks.end(), callback, SortCallbacks);

		m_callbacks.emplace(callbackIt, std::move(callback));
	}

	inline std::optional<std::pair<std::string, SpaceshipCore::CallbackArgFunction>> SpaceshipCore::PopCallback()
	{
		if (m_callbacks.empty())
			return {};

		Nz::UInt64 now = m_app->GetAppTime();
		if (m_callbacks.back().triggerTime >= now)
			return {};

		Callback callback = std::move(m_callbacks.back());
		m_callbacks.pop_back();

		auto it = m_pushedCallbacks.find(callback.callbackName);
		if (it != m_pushedCallbacks.end())
		{
			assert(it->second);
			it->second = false;
		}

		//std::cout << "Executing " << callback.callbackName << " (late by " << (now - callback.triggerTime) << "ms)" << std::endl;

		return std::make_pair(callback.callbackName, std::move(callback.argFunc));
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
