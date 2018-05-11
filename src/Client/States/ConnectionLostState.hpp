// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_CONNECTIONLOSTSTATE_HPP
#define EREWHON_CLIENT_STATES_CONNECTIONLOSTSTATE_HPP

#include <Client/ClientApplication.hpp>
#include <Client/ServerConnection.hpp>
#include <Client/States/AbstractState.hpp>
#include <Nazara/Core/Clock.hpp>
#include <Nazara/Graphics/TextSprite.hpp>
#include <Nazara/Renderer/RenderTarget.hpp>
#include <NDK/EntityOwner.hpp>
#include <NDK/State.hpp>
#include <NDK/World.hpp>

namespace ewn
{
	class ConnectionLostState final : public AbstractState
	{
		public:
			using AbstractState::AbstractState;
			~ConnectionLostState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets() override;

			void UpdateStatus(const Nz::String& status, const Nz::Color& color = Nz::Color::White, bool center = true);

			Ndk::EntityOwner m_statusText;
			Nz::TextSpriteRef m_statusSprite;
			float m_accumulator;
	};
}

#include <Client/States/ConnectionLostState.inl>

#endif // EREWHON_CLIENT_STATES_CONNECTIONLOSTSTATE_HPP
