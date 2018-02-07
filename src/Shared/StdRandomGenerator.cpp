// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/StdRandomGenerator.hpp>

namespace ewn
{
	bool StdRandomGenerator::operator()(void* ptr, std::size_t length)
	{
		Nz::UInt8* bytePtr = static_cast<Nz::UInt8*>(ptr);
		while (length > 0)
		{
			auto rndNumber = m_device();
			const Nz::UInt8* rndPtr = reinterpret_cast<const Nz::UInt8*>(&rndNumber);
			for (std::size_t i = 0; i < sizeof(rndNumber); ++i)
				*bytePtr++ = *rndPtr++;
		}

		return false;
	}
}
