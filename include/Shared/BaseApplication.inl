// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/BaseApplication.hpp>

namespace ewn
{
	inline Nz::UInt64 BaseApplication::GetAppTime() const
	{
		return m_appClock.GetMilliseconds();
	}

	inline const ConfigFile& BaseApplication::GetConfig() const
	{
		return m_config;
	}

	inline std::size_t BaseApplication::GetPeerPerReactor() const
	{
		return m_peerPerReactor;
	}

	inline std::size_t BaseApplication::GetReactorCount() const
	{
		return m_reactors.size();
	}

	inline bool BaseApplication::LoadConfig(const std::string& configFile)
	{
		return m_config.LoadFromFile(configFile);
	}

	inline const std::unique_ptr<NetworkReactor>& BaseApplication::GetReactor(std::size_t reactorId)
	{
		assert(reactorId < m_reactors.size());
		return m_reactors[reactorId];
	}
}
