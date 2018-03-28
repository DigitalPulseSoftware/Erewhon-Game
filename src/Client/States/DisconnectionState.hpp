// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_DISCONNECTIONSTATE_HPP
#define EREWHON_CLIENT_STATES_DISCONNECTIONSTATE_HPP

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
	class DisconnectionState final : public AbstractState
	{
		public:
			using AbstractState::AbstractState;
			~DisconnectionState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void CenterStatus();
			void OnServerDisconnected(ServerConnection* server, Nz::UInt32 data);
			void UpdateStatus(const Nz::String& status, const Nz::Color& color = Nz::Color::White, bool center = true);

			NazaraSlot(ServerConnection, OnDisconnected, m_onServerDisconnectedSlot);
			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			Ndk::EntityOwner m_statusText;
			Nz::TextSpriteRef m_statusSprite;
			bool m_disconnected;
			float m_accumulator;
			float m_timeout;
			unsigned int m_dotCounter;
	};
}

#include <Client/States/DisconnectionState.inl>

#endif // EREWHON_CLIENT_STATES_DISCONNECTIONSTATE_HPP
