// Copyright (C) 2018 Jérôme Leclercq
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
			inline NavigationModule(SpaceshipCore* core, const Ndk::EntityHandle& spaceship);
			~NavigationModule() = default;

			void FollowTarget(Nz::Int64 targetSignature);
			void FollowTarget(Nz::Int64 targetSignature, float triggerDistance);

			void MoveToPosition(const Nz::Vector3f& targetPos);
			void MoveToPosition(const Nz::Vector3f& targetPos, float triggerDistance);

			void Stop();

			void Register(Nz::LuaState& lua) override;

		private:
			void Initialize(Ndk::Entity* spaceship) override;

			static std::optional<Nz::LuaClass<NavigationModuleHandle>> s_binding;
	};
}

#include <Server/Modules/NavigationModule.inl>

#endif // EREWHON_SERVER_NAVIGATIONMODULE_HPP
