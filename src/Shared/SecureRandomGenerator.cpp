// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/SecureRandomGenerator.hpp>
#include <Shared/SystemRandomGenerator.hpp>
#include <iostream>

namespace ewn
{
	SecureRandomGenerator::SecureRandomGenerator()
	{
		try
		{
			m_secureGenerator = std::make_unique<SystemRandomGenerator>();
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to create secure random generator: " << e.what() << std::endl;
		}
	}

	bool SecureRandomGenerator::operator()(void* ptr, std::size_t length)
	{
		if (m_secureGenerator && (*m_secureGenerator)(ptr, length))
			return true;
		else
			return m_fallbackGenerator(ptr, length);
	}
}
