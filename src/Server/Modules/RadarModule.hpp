// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_RADARMODULE_HPP
#define EREWHON_SERVER_RADARMODULE_HPP

#include <Nazara/Lua/LuaClass.hpp>
#include <NDK/EntityList.hpp>
#include <Server/SpaceshipModule.hpp>
#include <Server/Scripting/LuaMathTypes.hpp>
#include <optional>
#include <unordered_map>

namespace ewn
{
	class RadarModule;

	using RadarModuleHandle = Nz::ObjectHandle<RadarModule>;

	class RadarModule : public SpaceshipModule, public Nz::HandledObject<RadarModule>
	{
		public:
			struct RangeInfo;
			struct TargetInfo;

			inline RadarModule(SpaceshipCore* core, const Ndk::EntityHandle& spaceship, float detectionRadius, std::size_t maxLockableTarget);
			~RadarModule() = default;

			inline const Ndk::EntityHandle& FindEntityBySignature(Nz::Int64 signature) const;
			void PushInstance(Nz::LuaState& lua) override;
			void RegisterModule(Nz::LuaClass<SpaceshipModule>& parentBinding, Nz::LuaState& lua) override;
			void Run(float elapsedTime) override;

			// Lua API
			inline void EnablePassiveScan(bool enable);

			std::optional<TargetInfo> GetTargetInfo(Nz::Int64 signature);

			inline bool IsPassiveScanEnabled() const;

			std::vector<RangeInfo> Scan();


			struct RangeInfo
			{
				ewn::LuaVec3 direction;
				Nz::Int64 signature;
				double emSignature;
				double distance;
				double size;
			};

			struct TargetInfo
			{
				ewn::LuaQuaternion rotation;
				ewn::LuaVec3 angularVelocity;
				ewn::LuaVec3 linearVelocity;
				ewn::LuaVec3 direction;
				Nz::Int64 signature;
				double emSignature;
				double distance;
				double size;
				double volume;
			};

		private:
			void PerformScan();
			inline void RemoveEntityFromRadius(Ndk::Entity* entity);

			std::size_t m_maxLockableTargets;
			std::unordered_map<Nz::Int64 /*signature*/, Ndk::EntityHandle /*entity*/> m_signatureToEntity;
			Ndk::EntityList m_entitiesInRadius;
			Ndk::EntityId m_lockedEntity;
			Nz::UInt64 m_lastPassiveScanTime;
			float m_detectionRadius;
			bool m_isPassiveScanEnabled;

			static std::optional<Nz::LuaClass<RadarModuleHandle>> s_binding;
	};
}

#include <Server/Modules/RadarModule.inl>

#endif // EREWHON_SERVER_RADARMODULE_HPP
