// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Modules/RadarModule.hpp>

namespace ewn
{
	inline RadarModule::RadarModule(const Ndk::EntityHandle & spaceship) :
	SpaceshipModule(spaceship),
	m_lastConeScanTime(0),
	m_lastTargetScanTime(0)
	{
	}
}

namespace Nz
{
	inline int LuaImplReplyVal(const LuaState& state, ewn::RadarModule* ptr, TypeTag<ewn::RadarModule*>)
	{
		state.PushInstance<ewn::RadarModuleHandle>("Radar", ptr);
		return 1;
	}

	inline int LuaImplReplyVal(const LuaState& state, ewn::RadarModule::ConeScanResult&& result, TypeTag<ewn::RadarModule::ConeScanResult>)
	{
		state.PushTable(0, 3);
		{
			state.PushField("id", result.id);
			state.PushField("type", std::move(result.type));
			state.PushField("pos", result.pos);
		}

		return 1;
	}

	inline int LuaImplReplyVal(const LuaState& state, ewn::RadarModule::ScanResult&& result, TypeTag<ewn::RadarModule::ScanResult>)
	{
		state.PushTable(0, 6);
		{
			state.PushField("name", std::move(result.name));
			state.PushField("type", std::move(result.type));
			state.PushField("angularVelocity", result.angularVelocity);
			state.PushField("linearVelocity", result.linearVelocity);
			state.PushField("position", result.position);
			state.PushField("rotation", result.rotation);
		}

		return 1;
	}
}
