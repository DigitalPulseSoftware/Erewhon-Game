// Copyright (C) 2017 Jérôme Leclercq
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
			struct ConeScanResult;
			using ConeScanResults = std::vector<ConeScanResult>;
			struct ScanResult;

			inline RadarModule(const Ndk::EntityHandle& spaceship);
			~RadarModule() = default;

			// Script functions
			bool IsConeScanReady() const;
			bool IsTargetScanReady() const;

			std::optional<ConeScanResults> ScanInCone(Nz::Vector3f direction);
			std::optional<ScanResult> ScanTarget(Ndk::EntityId targetId);

			// C++ functions
			void Register(Nz::LuaState& lua) override;

			struct ConeScanResult
			{
				Ndk::EntityId id;
				std::string type;
				ewn::LuaVec3 pos;
			};

			struct ScanResult
			{
				std::string name;
				std::string type;
				ewn::LuaVec3 position;
				ewn::LuaQuaternion rotation;
				ewn::LuaVec3 angularVelocity;
				ewn::LuaVec3 linearVelocity;
			};

		private:
			Ndk::EntityList m_visibleEntities;
			Nz::UInt64 m_lastConeScanTime;
			Nz::UInt64 m_lastTargetScanTime;

			static std::optional<Nz::LuaClass<RadarModuleHandle>> s_binding;
	};
}

#include <Server/Modules/RadarModule.inl>

#endif // EREWHON_SERVER_RADARMODULE_HPP
