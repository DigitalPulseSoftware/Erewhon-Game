// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_RADARMODULE_HPP
#define EREWHON_SERVER_RADARMODULE_HPP

#include <Nazara/Core/HandledObject.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Lua/LuaClass.hpp>
#include <NDK/EntityList.hpp>
#include <Server/SpaceshipModule.hpp>
#include <Server/Scripting/LuaMathTypes.hpp>
#include <optional>

namespace ewn
{
	class RadarModule;

	using RadarModuleHandle = Nz::ObjectHandle<RadarModule>;

	class RadarModule : public SpaceshipModule, public Nz::HandledObject<RadarModule>
	{
		public:
			struct TargetInfo;

			inline RadarModule(SpaceshipCore* core, const Ndk::EntityHandle& spaceship, float detectionRadius, std::size_t maxLockableTarget);
			~RadarModule() = default;

			// Script functions
			void ClearLockedTargets();

			inline void EnablePassiveScan(bool enable);

			std::optional<TargetInfo> GetTargetInfo(Ndk::EntityId targetId);

			inline bool IsPassiveScanEnabled() const;
			bool IsTargetLocked(Ndk::EntityId targetId) const;

			bool LockTarget(Ndk::EntityId targetId);
			void UnlockTarget(Ndk::EntityId targetId);

			// C++ functions
			void Register(Nz::LuaState& lua) override;
			void Run() override;

			struct TargetInfo
			{
				std::string name;
				ewn::LuaVec3 position;
				ewn::LuaQuaternion rotation;
				ewn::LuaVec3 angularVelocity;
				ewn::LuaVec3 linearVelocity;
			};

		private:
			void Initialize(Ndk::Entity* spaceship) override;
			void PerformScan();
			inline void RemoveEntityFromRadius(Ndk::Entity* entity);

			std::size_t m_maxLockableTargets;
			Ndk::EntityList m_entitiesInRadius;
			Nz::UInt64 m_lastPassiveScanTime;
			float m_detectionRadius;
			bool m_isPassiveScanEnabled;

			static std::optional<Nz::LuaClass<RadarModuleHandle>> s_binding;
	};
}

#include <Server/Modules/RadarModule.inl>

#endif // EREWHON_SERVER_RADARMODULE_HPP
