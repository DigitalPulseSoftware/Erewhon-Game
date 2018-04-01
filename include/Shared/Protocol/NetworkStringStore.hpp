// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_NETWORK_NETWORKSTRINGSTORE_HPP
#define EREWHON_SHARED_NETWORK_NETWORKSTRINGSTORE_HPP

#include <Shared/Protocol/Packets.hpp>
#include <hopstotch/hopscotch_map.h>
#include <optional>
#include <vector>

namespace ewn
{
	class NetworkStringStore
	{
		public:
			inline NetworkStringStore();
			~NetworkStringStore() = default;

			Packets::NetworkStrings BuildPacket(std::size_t firstId) const;

			inline void Clear();

			void FillStore(std::size_t firstId, std::vector<std::string> strings);

			inline const std::string& GetString(std::size_t id) const;
			inline std::size_t GetStringIndex(const std::string& string) const;

			inline std::size_t RegisterString(std::string string);

		private:
			tsl::hopscotch_map<std::string, std::size_t> m_stringMap;
			std::vector<std::string> m_strings;
	};
}

#include <Shared/Protocol/NetworkStringStore.inl>

#endif // EREWHON_SHARED_NETWORK_NETWORKSTRINGSTORE_HPP
