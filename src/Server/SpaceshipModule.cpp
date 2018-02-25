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

	void SpaceshipModule::PushCallback(Nz::UInt64 triggerTime, std::string callbackName)
	{
		m_core->PushCallback(triggerTime, std::move(callbackName));
	}
}
