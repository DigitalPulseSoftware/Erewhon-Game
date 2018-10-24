// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SCRIPTING_ARENA_INTERFACE_HPP
#define EREWHON_SCRIPTING_ARENA_INTERFACE_HPP

#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Lua/LuaClass.hpp>
#include <Nazara/Lua/LuaState.hpp>
#include <Server/Player.hpp>

namespace ewn
{
	class Arena;

	class ArenaInterface
	{
		public:
			ArenaInterface() = delete;
			~ArenaInterface() = delete;

			static void Register(Nz::LuaState& lua);

			static bool Initialize();
			static void Uninitialize();

		private:
			static void BindArena();
			static void BindPlayer();

			static Nz::LuaClass<Arena*> s_arenaBinding;
			static Nz::LuaClass<PlayerHandle> s_playerBinding;
	};
}

#include <Server/Scripting/ArenaInterface.inl>

#endif // EREWHON_SCRIPTING_ARENA_INTERFACE_HPP
