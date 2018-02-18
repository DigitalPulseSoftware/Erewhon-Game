// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_ENGINEMODULE_HPP
#define EREWHON_SERVER_ENGINEMODULE_HPP

#include <Nazara/Core/HandledObject.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Lua/LuaClass.hpp>
#include <Nazara/Math/Vector3.hpp>
#include <Server/SpaceshipModule.hpp>
#include <optional>

namespace ewn
{
	class EngineModule;
	class ServerApplication;

	using EngineModuleHandle = Nz::ObjectHandle<EngineModule>;

	class EngineModule : public SpaceshipModule, public Nz::HandledObject<EngineModule>
	{
		public:
			inline EngineModule(const Ndk::EntityHandle& spaceship);
			~EngineModule() = default;

			void Impulse(Nz::Vector3f impulse, float duration);

			void Register(Nz::LuaState& lua) override;

		private:
			static std::optional<Nz::LuaClass<EngineModuleHandle>> s_binding;
	};
}

#include <Server/Modules/EngineModule.inl>

#endif // EREWHON_SERVER_ENGINEMODULE_HPP
