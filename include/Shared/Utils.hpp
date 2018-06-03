// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_UTILS_HPP
#define EREWHON_SHARED_UTILS_HPP

#include <Nazara/Math/Vector3.hpp>
#include <type_traits>

namespace ewn
{
	template<typename T>
	struct AlwaysFalse : std::false_type {};

	Nz::Vector3f DampenedString(const Nz::Vector3f& currentPos, const Nz::Vector3f& targetPos, float frametime, float springStrength = 3.f);
}

#endif // EREWHON_SHARED_UTILS_HPP
