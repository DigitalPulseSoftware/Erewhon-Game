// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/SpaceshipModule.hpp>
#include <Server/SpaceshipCore.hpp>

namespace ewn
{
	SpaceshipModule::~SpaceshipModule() = default;

	void SpaceshipModule::PushCallback(std::string callbackName)
	{
		m_core->PushCallback(std::move(callbackName));
	}
}
