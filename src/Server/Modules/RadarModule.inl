// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Modules/RadarModule.hpp>

namespace ewn
{
	inline RadarModule::RadarModule(SpaceshipCore* core, const Ndk::EntityHandle & spaceship, float detectionRadius, std::size_t maxLockableTarget) :
	SpaceshipModule(ModuleType::Radar, core, spaceship, true),
	m_maxLockableTargets(maxLockableTarget),
	m_detectionRadius(detectionRadius),
	m_isPassiveScanEnabled(true)
	{
	}

	inline const Ndk::EntityHandle& RadarModule::FindEntityBySignature(Nz::Int64 signature) const
	{
		auto signatureIt = m_signatureToEntity.find(signature);
		if (signatureIt == m_signatureToEntity.end())
			return Ndk::EntityHandle::InvalidHandle;

		return signatureIt->second;
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

	inline int LuaImplReplyVal(const LuaState& state, ewn::RadarModule::RangeInfo&& value, TypeTag<ewn::RadarModule::RangeInfo>)
	{
		state.PushTable(0, 4);
		{
			state.PushField("direction", value.direction);
			state.PushField("distance", value.distance);
			state.PushField("emSignature", value.emSignature);
			state.PushField("signature", value.signature);
			state.PushField("size", value.size);
		}

		return 1;
	}

	inline int LuaImplReplyVal(const LuaState& state, ewn::RadarModule::TargetInfo&& value, TypeTag<ewn::RadarModule::TargetInfo>)
	{
		state.PushTable(0, 8);
		{
			state.PushField("angularVelocity", value.angularVelocity);
			state.PushField("direction", value.direction);
			state.PushField("distance", value.distance);
			state.PushField("emSignature", value.emSignature);
			state.PushField("linearVelocity", value.linearVelocity);
			state.PushField("rotation", value.rotation);
			state.PushField("signature", value.signature);
			state.PushField("size", value.size);
			state.PushField("volume", value.volume);
		}

		return 1;
	}
}
