// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Modules/RadarModule.hpp>

namespace ewn
{
	inline RadarModule::RadarModule(SpaceshipCore* core, const Ndk::EntityHandle & spaceship, float detectionRadius, std::size_t maxLockableTarget) :
	SpaceshipModule(core, spaceship, true),
	m_maxLockableTargets(maxLockableTarget),
	m_detectionRadius(detectionRadius),
	m_isPassiveScanEnabled(true)
	{
	}

	inline void RadarModule::EnablePassiveScan(bool enable)
	{
		m_isPassiveScanEnabled = enable;
	}

	inline bool RadarModule::IsPassiveScanEnabled() const
	{
		return m_isPassiveScanEnabled;
	}

	inline void RadarModule::RemoveEntityFromRadius(Ndk::Entity* entity)
	{
		m_entitiesInRadius.Remove(entity);
	}
}

namespace Nz
{
	inline int LuaImplReplyVal(const LuaState& state, ewn::RadarModule* ptr, TypeTag<ewn::RadarModule*>)
	{
		state.PushInstance<ewn::RadarModuleHandle>("Radar", ptr);
		return 1;
	}

	inline int LuaImplReplyVal(const LuaState& state, ewn::RadarModule::TargetInfo&& result, TypeTag<ewn::RadarModule::TargetInfo>)
	{
		state.PushTable(0, 4);
		{
			state.PushField("angularVelocity", result.angularVelocity);
			state.PushField("linearVelocity", result.linearVelocity);
			state.PushField("position", result.position);
			state.PushField("rotation", result.rotation);
		}

		return 1;
	}
}
