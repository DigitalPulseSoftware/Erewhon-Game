// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_SCRIPTCOMPONENT_HPP
#define EREWHON_SERVER_SCRIPTCOMPONENT_HPP

#include <Nazara/Lua/LuaInstance.hpp>
#include <NDK/Component.hpp>
#include <NDK/EntityList.hpp>
#include <Server/SpaceshipCore.hpp>
#include <optional>

namespace ewn
{
	class ServerApplication;

	class ScriptComponent : public Ndk::Component<ScriptComponent>
	{
		public:
			ScriptComponent(std::string scriptName);
			ScriptComponent(const ScriptComponent& component);

			void Run(ServerApplication* app, float elapsedTime);

			static Ndk::ComponentIndex componentIndex;

		private:
			void OnAttached() override;
			void OnDetached() override;

			std::optional<SpaceshipCore> m_core;
			std::string m_scriptName;
			Nz::LuaInstance m_instance;
	};
}

#include <Server/Components/ScriptComponent.inl>

#endif // EREWHON_SERVER_SCRIPTCOMPONENT_HPP
