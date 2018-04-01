// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_STD_RANDOM_GENERATOR_HPP
#define EREWHON_SHARED_STD_RANDOM_GENERATOR_HPP

#include <Shared/RandomGenerator.hpp>
#include <random>

namespace ewn
{
	class StdRandomGenerator : public RandomGenerator
	{
		public:
			StdRandomGenerator() = default;
			~StdRandomGenerator() = default;

			bool operator()(void* ptr, std::size_t length) override;

		private:
			std::random_device m_device;
	};
}

#include <Shared/StdRandomGenerator.inl>

#endif // EREWHON_SHARED_STD_RANDOM_GENERATOR_HPP
