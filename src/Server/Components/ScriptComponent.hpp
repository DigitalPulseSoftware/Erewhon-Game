// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_SCRIPTCOMPONENT_HPP
#define EREWHON_SERVER_SCRIPTCOMPONENT_HPP

#include <Nazara/Lua/LuaInstance.hpp>
#include <NDK/Component.hpp>
#include <NDK/EntityList.hpp>
#include <Shared/Enums.hpp>
#include <Server/SpaceshipCore.hpp>
#include <optional>

namespace ewn
{
	class ServerApplication;

	class ScriptComponent : public Ndk::Component<ScriptComponent>
	{
		public:
			ScriptComponent();
			ScriptComponent(const ScriptComponent& component);

			bool Execute(Nz::String script, Nz::String* lastError);

			inline bool HasValidScript() const;

			bool Run(ServerApplication* app, float elapsedTime, Nz::String* lastError = nullptr);

			void SendMessage(BotMessageType messageType, Nz::String message);

			static Ndk::ComponentIndex componentIndex;

		private:
			void OnAttached() override;
			void OnDetached() override;

			std::optional<SpaceshipCore> m_core;
			Nz::UInt64 m_lastMessageTime;
			Nz::LuaInstance m_instance;
			Nz::String m_script;
	};
}

#include <Server/Components/ScriptComponent.inl>

#endif // EREWHON_SERVER_SCRIPTCOMPONENT_HPP
