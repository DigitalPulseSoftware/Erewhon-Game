// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_RANDOM_GENERATOR_HPP
#define EREWHON_SHARED_RANDOM_GENERATOR_HPP

#include <Nazara/Prerequisites.hpp>

namespace ewn
{
	class RandomGenerator
	{
		public:
			RandomGenerator() = default;
			virtual ~RandomGenerator();

			virtual bool operator()(void* ptr, std::size_t length) = 0;
	};
}

#include <Shared/RandomGenerator.inl>

#endif // EREWHON_SHARED_RANDOM_GENERATOR_HPP
