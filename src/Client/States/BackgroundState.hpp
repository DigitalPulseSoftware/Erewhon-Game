// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_BACKGROUNDSTATE_HPP
#define EREWHON_CLIENT_STATES_BACKGROUNDSTATE_HPP

#include <Client/States/StateData.hpp>
#include <Nazara/Renderer/RenderTarget.hpp>
#include <Nazara/Renderer/Texture.hpp>
#include <NDK/State.hpp>
#include <NDK/World.hpp>

namespace ewn
{
	class BackgroundState final : public Ndk::State
	{
		public:
			BackgroundState(StateData& stateData);
			~BackgroundState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			StateData m_stateData;
			Nz::TextureRef m_backgroundCubemap;
	};
}

#include <Client/States/BackgroundState.inl>

#endif // EREWHON_CLIENT_STATES_BACKGROUNDSTATE_HPP
