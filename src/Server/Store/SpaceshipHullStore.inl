// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Store/SpaceshipHullStore.hpp>
#include <cassert>

namespace ewn
{
	inline SpaceshipHullStore::SpaceshipHullStore() :
	DatabaseStore("LoadSpaceshipHulls")
	{
	}
}