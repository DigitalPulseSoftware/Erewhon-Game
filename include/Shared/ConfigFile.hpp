// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_CONFIGFILE_HPP
#define EREWHON_SHARED_CONFIGFILE_HPP

#include <unordered_map>
#include <variant>

namespace ewn
{
	class ConfigFile
	{
		public:
			ConfigFile() = default;
			~ConfigFile() = default;

			inline bool GetBoolOption(const std::string& optionName) const;
			inline double GetFloatOption(const std::string& optionName) const;
			inline long long GetIntegerOption(const std::string& optionName) const;
			inline const std::string& GetStringOption(const std::string& optionName) const;

			bool LoadFromFile(const std::string& fileName);

			inline void RegisterBoolOption(std::string optionName);
			inline void RegisterFloatOption(std::string optionName);
			inline void RegisterFloatOption(std::string optionName, double minBounds, double maxBounds);
			inline void RegisterIntegerOption(std::string optionName);
			inline void RegisterIntegerOption(std::string optionName, long long minBounds, long long maxBounds);
			inline void RegisterStringOption(std::string optionName);

		private:
			struct BoolOption
			{
				bool value;
			};

			struct FloatOption
			{
				double maxBounds;
				double minBounds;
				double value;
			};

			struct IntegerOption
			{
				long long maxBounds;
				long long minBounds;
				long long value;
			};

			struct StringOption
			{
				std::string value;
			};

			using ConfigOption = std::variant<BoolOption, FloatOption, IntegerOption, StringOption>;

			inline void RegisterOption(std::string optionName, ConfigOption option);

			std::unordered_map<std::string, ConfigOption> m_options;
	};
}

#include <Shared/ConfigFile.inl>

#endif // EREWHON_SHARED_CONFIGFILE_HPP
