// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/SystemRandomGenerator.hpp>
#include <iostream>
#include <stdexcept>

namespace ewn
{
	SystemRandomGenerator::SystemRandomGenerator()
	{
		bool success = false;
#ifdef NAZARA_PLATFORM_WINDOWS
		success = CryptAcquireContext(&m_provider, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT);
#else
		m_handle.open("/dev/urandom");
		success = m_handle.is_open();
#endif

		if (!success)
			throw std::runtime_error("Failed to open random generator context");
	}

	SystemRandomGenerator::~SystemRandomGenerator()
	{
#ifdef NAZARA_PLATFORM_WINDOWS
		if (!CryptReleaseContext(m_provider, 0))
			std::cerr << "Failed to free HCRYPTPROV: " << ::GetLastError() << std::endl;
#endif
	}

	bool SystemRandomGenerator::operator()(void* ptr, std::size_t length)
	{
#ifdef NAZARA_PLATFORM_WINDOWS
		return CryptGenRandom(m_provider, DWORD(length), static_cast<BYTE*>(ptr)) == TRUE;
#else
		m_handle.read(static_cast<char*>(ptr), length);
		return m_handle.gcount() == length;
#endif
	}
}
