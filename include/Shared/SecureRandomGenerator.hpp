// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_SECURE_RANDOM_GENERATOR_HPP
#define EREWHON_SHARED_SECURE_RANDOM_GENERATOR_HPP

#include <Nazara/Prerequisites.hpp>
#include <Shared/RandomGenerator.hpp>
#include <Shared/StdRandomGenerator.hpp>
#include <memory>

namespace ewn
{
	class SecureRandomGenerator : public RandomGenerator
	{
		public:
			SecureRandomGenerator();
			~SecureRandomGenerator() = default;

			bool operator()(void* ptr, std::size_t length) override;

		private:
			std::unique_ptr<RandomGenerator> m_secureGenerator;
			StdRandomGenerator m_fallbackGenerator;
	};
}

#include <Shared/RandomGenerator.inl>

#endif // EREWHON_SHARED_SECURE_RANDOM_GENERATOR_HPP
