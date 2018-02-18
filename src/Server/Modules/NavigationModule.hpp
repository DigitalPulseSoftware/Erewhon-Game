// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_NAVIGATIONMODULE_HPP
#define EREWHON_SERVER_NAVIGATIONMODULE_HPP

#include <Nazara/Core/HandledObject.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Lua/LuaClass.hpp>
#include <Nazara/Math/Vector3.hpp>
#include <Server/SpaceshipModule.hpp>
#include <optional>

namespace ewn
{
	class NavigationModule;

	using NavigationModuleHandle = Nz::ObjectHandle<NavigationModule>;

	class NavigationModule : public SpaceshipModule, public Nz::HandledObject<NavigationModule>
	{
		public:
			inline NavigationModule(const Ndk::EntityHandle& spaceship);
			~NavigationModule() = default;

			void FollowTarget(Ndk::EntityId targetId);
			void MoveToPosition(const Nz::Vector3f& targetPos);
			void Stop();

			void Register(Nz::LuaState& lua) override;

		private:
			static std::optional<Nz::LuaClass<NavigationModuleHandle>> s_binding;
	};
}

#include <Server/Modules/NavigationModule.inl>

#endif // EREWHON_SERVER_NAVIGATIONMODULE_HPP
