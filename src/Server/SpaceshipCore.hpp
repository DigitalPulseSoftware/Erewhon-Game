// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_SPACESHIPCORE_HPP
#define EREWHON_SERVER_SPACESHIPCORE_HPP

#include <Nazara/Core/HandledObject.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Lua/LuaClass.hpp>
#include <NDK/Entity.hpp>
#include <Server/Scripting/LuaMathTypes.hpp>
#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

namespace ewn
{
	class SpaceshipCore;
	class SpaceshipModule;

	using SpaceshipCoreHandle = Nz::ObjectHandle<SpaceshipCore>;

	class SpaceshipCore : public Nz::HandledObject<SpaceshipCore>
	{
		public:
			inline SpaceshipCore(const Ndk::EntityHandle& spaceship);
			SpaceshipCore(const SpaceshipCore&) = delete;
			~SpaceshipCore();

			inline void AddModule(std::shared_ptr<SpaceshipModule> modulePtr);

			LuaVec3 GetAngularVelocity() const;
			float GetIntegrity() const;
			LuaVec3 GetLinearVelocity() const;
			LuaVec3 GetPosition() const;
			LuaQuaternion GetRotation() const;

			void Register(Nz::LuaState& lua);

			inline void PushCallback(std::string callbackName);
			inline void PushCallback(Nz::UInt64 triggerTime, std::string callbackName);
			inline std::optional<std::string> PopCallback();

			SpaceshipCore& operator=(const SpaceshipCore&) = delete;

		private:
			struct Callback
			{
				Nz::UInt64 triggerTime;
				std::string callbackName;
			};

			std::unordered_map<std::string, bool> m_pushedCallbacks;
			std::vector<std::shared_ptr<SpaceshipModule>> m_modules;
			std::vector<Callback> m_callbacks;
			Ndk::EntityHandle m_spaceship;

			static std::optional<Nz::LuaClass<SpaceshipCoreHandle>> s_binding;
	};
}

#include <Server/SpaceshipCore.inl>

#endif // EREWHON_SERVER_SPACESHIPCORE_HPP
