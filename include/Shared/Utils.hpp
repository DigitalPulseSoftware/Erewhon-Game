// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_UTILS_HPP
#define EREWHON_SHARED_UTILS_HPP

#include <type_traits>

namespace ewn
{
	template<typename T>
	struct AlwaysFalse : std::false_type {};
}

#endif // EREWHON_SHARED_UTILS_HPP
