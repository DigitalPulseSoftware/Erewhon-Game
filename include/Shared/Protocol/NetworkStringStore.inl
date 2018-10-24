// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Protocol/NetworkStringStore.hpp>
#include <cassert>

namespace ewn
{
	inline NetworkStringStore::NetworkStringStore()
	{
		RegisterString(""); //< Force #0 to be empty string
	}

	inline void NetworkStringStore::Clear()
	{
		m_stringMap.clear();
		m_strings.clear();
	}

	inline const std::string& NetworkStringStore::GetString(Nz::UInt32 id) const
	{
		assert(id < m_strings.size());
		return m_strings[id];
	}

	inline Nz::UInt32 NetworkStringStore::GetStringIndex(const std::string& string) const
	{
		auto it = m_stringMap.find(string);
		assert(it != m_stringMap.end());

		return it->second;
	}

	inline Nz::UInt32 NetworkStringStore::RegisterString(std::string string)
	{
		// Try to find string first, it may have been registered already
		auto it = m_stringMap.find(string);
		if (it != m_stringMap.end())
			return it->second;

		// String does not exist; insert it
		Nz::UInt32 stringId = static_cast<Nz::UInt32>(m_strings.size());
		m_stringMap.emplace(string, stringId);
		m_strings.emplace_back(std::move(string));

		return stringId;
	}
}
