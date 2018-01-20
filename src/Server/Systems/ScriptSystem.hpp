// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_SCRIPTSYSTEM_HPP
#define EREWHON_SERVER_SCRIPTSYSTEM_HPP

#include <NDK/System.hpp>

namespace ewn
{
	class Arena;
	class ServerApplication;

	class ScriptSystem : public Ndk::System<ScriptSystem>
	{
		public:
			ScriptSystem(ServerApplication* app, Arena* arena);
			~ScriptSystem() = default;

			static Ndk::SystemIndex systemIndex;

		private:
			void OnUpdate(float elapsedTime) override;

			Arena* m_arena;
			ServerApplication* m_app;
	};
}

#include <Server/Systems/ScriptSystem.inl>

#endif // EREWHON_SERVER_SCRIPTSYSTEM_HPP
