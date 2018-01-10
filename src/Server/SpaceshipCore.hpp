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
#include <Server/SpaceshipModule.hpp>
#include <optional>
#include <vector>

namespace ewn
{
	class SpaceshipCore;

	using SpaceshipCoreHandle = Nz::ObjectHandle<SpaceshipCore>;

	class SpaceshipCore : public Nz::HandledObject<SpaceshipCore>
	{
		public:
			inline SpaceshipCore(const Ndk::EntityHandle& spaceship);
			~SpaceshipCore() = default;

			float GetIntegrity() const;

			void Register(Nz::LuaState& lua);

		private:
			std::vector<std::unique_ptr<SpaceshipModule>> m_modules;
			Ndk::EntityHandle m_spaceship;

			static std::optional<Nz::LuaClass<SpaceshipCore>> s_binding;
	};
}

#include <Server/SpaceshipCore.inl>

#endif // EREWHON_SERVER_SPACESHIPCORE_HPP
