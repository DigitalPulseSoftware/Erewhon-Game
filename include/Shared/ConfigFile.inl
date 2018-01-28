// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/ConfigFile.hpp>
#include <Nazara/Core/Error.hpp>
#include <limits>

namespace ewn
{
	inline bool ConfigFile::GetBoolOption(const std::string& optionName) const
	{
		return std::get<BoolOption>(m_options.at(optionName)).value;
	}

	inline double ConfigFile::GetFloatOption(const std::string& optionName) const
	{
		return std::get<FloatOption>(m_options.at(optionName)).value;
	}

	inline long long ConfigFile::GetIntegerOption(const std::string& optionName) const
	{
		return std::get<IntegerOption>(m_options.at(optionName)).value;
	}

	inline const std::string& ConfigFile::GetStringOption(const std::string& optionName) const
	{
		return std::get<StringOption>(m_options.at(optionName)).value;
	}

	inline void ConfigFile::RegisterBoolOption(std::string optionName)
	{
		RegisterOption(std::move(optionName), BoolOption{});
	}

	inline void ConfigFile::RegisterFloatOption(std::string optionName)
	{
		RegisterFloatOption(std::move(optionName), -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
	}

	inline void ConfigFile::RegisterFloatOption(std::string optionName, double minBounds, double maxBounds)
	{
		FloatOption floatOption;
		floatOption.maxBounds = maxBounds;
		floatOption.minBounds = minBounds;

		RegisterOption(std::move(optionName), std::move(floatOption));
	}

	inline void ConfigFile::RegisterIntegerOption(std::string optionName)
	{
		RegisterIntegerOption(std::move(optionName), std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
	}

	inline void ConfigFile::RegisterIntegerOption(std::string optionName, long long minBounds, long long maxBounds)
	{
		IntegerOption intOption;
		intOption.maxBounds = maxBounds;
		intOption.minBounds = minBounds;

		RegisterOption(std::move(optionName), std::move(intOption));
	}

	inline void ConfigFile::RegisterStringOption(std::string optionName)
	{
		RegisterOption(std::move(optionName), StringOption{});
	}

	inline void ConfigFile::RegisterOption(std::string optionName, ConfigOption option)
	{
		NazaraAssert(m_options.find(optionName) == m_options.end(), "Option already exists");

		m_options.emplace(std::move(optionName), std::move(option));
	}
}
