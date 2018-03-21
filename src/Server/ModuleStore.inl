// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ModuleStore.hpp>
#include <cassert>

namespace ewn
{
	inline ModuleStore::ModuleStore() :
	m_isLoaded(false)
	{
		BuildFactory();
	}

	inline bool ModuleStore::IsLoaded() const
	{
		return m_isLoaded;
	}

	inline void ModuleStore::RegisterModule(std::string className, DecodeClassInfoFunction decodeFunc, FactoryFunction factoryFunc)
	{
		assert(m_factory.find(className) == m_factory.end());

		FactoryData& factoryData = m_factory.emplace(std::move(className), FactoryData()).first->second;
		factoryData.decodeFunc = std::move(decodeFunc);
		factoryData.factoryFunc = std::move(factoryFunc);
	}
}
