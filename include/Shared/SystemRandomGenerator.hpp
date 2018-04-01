// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_SYSTEM_RANDOM_GENERATOR_HPP
#define EREWHON_SHARED_SYSTEM_RANDOM_GENERATOR_HPP

#include <Nazara/Prerequisites.hpp>
#include <Shared/RandomGenerator.hpp>

#ifdef NAZARA_PLATFORM_WINDOWS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>
#else
#include <fstream>
#endif

namespace ewn
{
	class SystemRandomGenerator : public RandomGenerator
	{
		public:
			SystemRandomGenerator();
			~SystemRandomGenerator();

			bool operator()(void* ptr, std::size_t length) override;

		private:
#ifdef NAZARA_PLATFORM_WINDOWS
			HCRYPTPROV m_provider;
#else
			std::ifstream m_handle;
#endif
	};
}

#include <Shared/SystemRandomGenerator.inl>

#endif // EREWHON_SHARED_SYSTEM_RANDOM_GENERATOR_HPP
