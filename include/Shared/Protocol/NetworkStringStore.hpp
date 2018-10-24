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

			Packets::NetworkStrings BuildPacket(Nz::UInt32 firstId) const;

			inline void Clear();

			void FillStore(Nz::UInt32 firstId, std::vector<std::string> strings);

			inline const std::string& GetString(Nz::UInt32 id) const;
			inline Nz::UInt32 GetStringIndex(const std::string& string) const;

			inline Nz::UInt32 RegisterString(std::string string);

		private:
			tsl::hopscotch_map<std::string, Nz::UInt32> m_stringMap;
			std::vector<std::string> m_strings;
	};
}

#include <Shared/Protocol/NetworkStringStore.inl>

#endif // EREWHON_SHARED_NETWORK_NETWORKSTRINGSTORE_HPP
