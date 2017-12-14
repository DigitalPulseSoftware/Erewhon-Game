// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_CONNECTIONLOSTSTATE_HPP
#define EREWHON_CLIENT_STATES_CONNECTIONLOSTSTATE_HPP

#include <Nazara/Core/Clock.hpp>
#include <Nazara/Graphics/TextSprite.hpp>
#include <Nazara/Renderer/RenderTarget.hpp>
#include <NDK/EntityOwner.hpp>
#include <NDK/State.hpp>
#include <NDK/World.hpp>
#include <Client/ClientApplication.hpp>
#include <Client/States/StateData.hpp>
#include <Client/ServerConnection.hpp>

namespace ewn
{
	class ConnectionLostState final : public Ndk::State
	{
		public:
			inline ConnectionLostState(StateData& stateData);
			~ConnectionLostState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void CenterStatus();
			void OnServerDisconnected(ServerConnection* server, Nz::UInt32 data);
			void UpdateStatus(const Nz::String& status, const Nz::Color& color = Nz::Color::White, bool center = true);

			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			StateData& m_stateData;
			Ndk::EntityOwner m_statusText;
			Nz::TextSpriteRef m_statusSprite;
			float m_accumulator;
	};
}

#include <Client/States/ConnectionLostState.inl>

#endif // EREWHON_CLIENT_STATES_CONNECTIONLOSTSTATE_HPP
