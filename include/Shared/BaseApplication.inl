// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/BaseApplication.hpp>

namespace ewn
{
	inline Nz::UInt64 BaseApplication::GetAppTime()
	{
		return m_appTime.load(std::memory_order_relaxed);
	}

	inline ConfigFile& BaseApplication::GetConfig()
	{
		return m_config;
	}

	inline const ConfigFile& BaseApplication::GetConfig() const
	{
		return m_config;
	}

	inline std::size_t BaseApplication::GetReactorCount() const
	{
		return m_reactors.size();
	}

	inline bool BaseApplication::LoadConfig(const std::string& configFile)
	{
		if (m_config.LoadFromFile(configFile))
		{
			OnConfigLoaded(m_config);
			return true;
		}
		else
			return false;
	}

	inline std::size_t BaseApplication::AddReactor(std::unique_ptr<NetworkReactor> reactor)
	{
		m_reactors.emplace_back(std::move(reactor));
		return m_reactors.size() - 1;
	}

	inline void BaseApplication::ClearReactors()
	{
		m_reactors.clear();
	}

	inline const std::unique_ptr<NetworkReactor>& BaseApplication::GetReactor(std::size_t reactorId)
	{
		assert(reactorId < m_reactors.size());
		return m_reactors[reactorId];
	}
}
