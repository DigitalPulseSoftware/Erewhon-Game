// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Store/ModuleStore.hpp>
#include <cassert>

namespace ewn
{
	inline ModuleStore::ModuleStore() :
	DatabaseStore("LoadModules")
	{
		BuildFactory();
	}

	inline std::size_t ModuleStore::GetEntryByName(const std::string& entryName) const
	{
		auto it = m_moduleIndices.find(entryName);
		if (it != m_moduleIndices.end())
			return it->second;
		else
			return InvalidEntryId;
	}

	inline std::size_t ModuleStore::GetEntryCount() const
	{
		return m_moduleInfos.size();
	}

	inline const std::string& ModuleStore::GetEntryClassName(std::size_t entryId) const
	{
		assert(IsEntryLoaded(entryId));
		return m_moduleInfos[entryId].className;
	}

	inline const std::string& ModuleStore::GetEntryDescription(std::size_t entryId) const
	{
		assert(IsEntryLoaded(entryId));
		return m_moduleInfos[entryId].description;
	}

	inline const std::string& ModuleStore::GetEntryName(std::size_t entryId) const
	{
		assert(IsEntryLoaded(entryId));
		return m_moduleInfos[entryId].name;
	}

	inline ModuleType ModuleStore::GetEntryType(std::size_t entryId) const
	{
		assert(IsEntryLoaded(entryId));
		return m_moduleInfos[entryId].type;
	}

	inline bool ModuleStore::IsEntryLoaded(std::size_t entryId) const
	{
		assert(entryId < m_moduleInfos.size());
		return m_moduleInfos[entryId].isLoaded;
	}

	inline void ModuleStore::RegisterModule(std::string className, DecodeClassInfoFunction decodeFunc, FactoryFunction factoryFunc)
	{
		assert(m_factory.find(className) == m_factory.end());

		FactoryData& factoryData = m_factory.emplace(std::move(className), FactoryData()).first->second;
		factoryData.decodeFunc = std::move(decodeFunc);
		factoryData.factoryFunc = std::move(factoryFunc);
	}
}
