// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_CONNECTIONSTATE_HPP
#define EREWHON_CLIENT_STATES_CONNECTIONSTATE_HPP

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
	class ConnectionState final : public Ndk::State
	{
		public:
			inline ConnectionState(StateData& stateData);
			~ConnectionState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void CenterStatus();
			void OnServerConnected(ServerConnection* server, Nz::UInt32 data);
			void OnServerDisconnected(ServerConnection* server, Nz::UInt32 data);
			void UpdateStatus(const Nz::String& status, const Nz::Color& color = Nz::Color::White, bool center = true);

			NazaraSlot(ServerConnection, OnConnected, m_onServerConnectedSlot);
			NazaraSlot(ServerConnection, OnDisconnected, m_onServerDisconnectedSlot);
			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			StateData& m_stateData;
			Ndk::EntityOwner m_statusText;
			Nz::Ternary m_connected;
			Nz::TextSpriteRef m_statusSprite;
			float m_accumulator;
			unsigned int m_counter;
	};
}

#include <Client/States/ConnectionState.inl>

#endif // EREWHON_CLIENT_STATES_CONNECTIONSTATE_HPP
