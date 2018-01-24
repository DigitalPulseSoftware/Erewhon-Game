// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_LOGINSTATE_HPP
#define EREWHON_CLIENT_STATES_LOGINSTATE_HPP

#include <Client/States/StateData.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>

namespace ewn
{
	class LoginState final : public Ndk::State
	{
		public:
			inline LoginState(StateData& stateData);
			~LoginState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets();

			void OnConnected(ServerConnection* server, Nz::UInt32 data);
			void OnConnectionPressed();
			void OnDisconnected(ServerConnection* server, Nz::UInt32 data);

			void SendLoginPacket();

			void UpdateStatus(const Nz::String& status, const Nz::Color& color = Nz::Color::White);

			NazaraSlot(ServerConnection, OnConnected,    m_onConnectedSlot);
			NazaraSlot(ServerConnection, OnDisconnected, m_onDisconnectedSlot);
			NazaraSlot(ServerConnection, OnLoginFailure, m_onLoginFailureSlot);
			NazaraSlot(ServerConnection, OnLoginSuccess, m_onLoginSuccess);

			StateData& m_stateData;
			Ndk::ButtonWidget* m_connectionButton;
			Ndk::CheckboxWidget* m_rememberCheckbox;
			Ndk::LabelWidget* m_loginLabel;
			Ndk::LabelWidget* m_passwordLabel;
			Ndk::LabelWidget* m_statusLabel;
			Ndk::TextAreaWidget* m_loginArea;
			Ndk::TextAreaWidget* m_passwordArea;
			bool m_loginSucceeded;
			bool m_isLoggingIn;
			float m_loginAccumulator;
	};
}

#include <Client/States/LoginState.inl>

#endif // EREWHON_CLIENT_STATES_LOGINSTATE_HPP
