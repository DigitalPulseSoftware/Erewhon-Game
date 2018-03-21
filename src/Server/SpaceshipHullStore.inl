// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/SpaceshipHullStore.hpp>
#include <cassert>

namespace ewn
{
	inline SpaceshipHullStore::SpaceshipHullStore() :
	m_isLoaded(false)
	{
		//BuildFactory();
	}

	inline bool SpaceshipHullStore::IsLoaded() const
	{
		return m_isLoaded;
	}
}
