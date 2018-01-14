// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_ARENACOMPONENT_HPP
#define EREWHON_SERVER_ARENACOMPONENT_HPP

#include <NDK/Component.hpp>
#include <Server/Arena.hpp>

namespace ewn
{
	class ArenaComponent : public Ndk::Component<ArenaComponent>
	{
		public:
			inline ArenaComponent(Arena& arena);

			inline Arena& GetArena();

			inline operator Arena&();

			static Ndk::ComponentIndex componentIndex;

		private:
			Arena& m_arena;
	};
}

#include <Server/Components/ArenaComponent.inl>

#endif // EREWHON_SERVER_ARENACOMPONENT_HPP
