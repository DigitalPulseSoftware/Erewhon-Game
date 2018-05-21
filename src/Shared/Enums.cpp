// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Enums.hpp>
#include <cassert>

namespace ewn
{
	const char* EnumToString(ModuleType moduleType)
	{
		switch (moduleType)
		{
			case ModuleType::Communications:
				return "Communications";

			case ModuleType::Engine:
				return "Engine";

			case ModuleType::Navigation:
				return "Navigation";

			case ModuleType::Radar:
				return "Radar";

			case ModuleType::Weapon:
				return "Weapon";
		}

		assert(!"Unhandled enum value");
		return nullptr;
	}
}
