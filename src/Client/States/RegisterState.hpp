// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_REGISTERSTATE_HPP
#define EREWHON_CLIENT_STATES_REGISTERSTATE_HPP

#include <Client/States/StateData.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>

namespace ewn
{
	class RegisterState final : public Ndk::State
	{
		public:
			inline RegisterState(StateData& stateData);
			~RegisterState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets();

			void OnCancelPressed();
			void OnConnected(ServerConnection* server, Nz::UInt32 data);
			void OnDisconnected(ServerConnection* server, Nz::UInt32 data);
			void OnRegisterPressed();

			void SendRegisterPacket();

			void UpdateStatus(const Nz::String& status, const Nz::Color& color = Nz::Color::White);

			NazaraSlot(ServerConnection, OnConnected,    m_onConnectedSlot);
			NazaraSlot(ServerConnection, OnDisconnected, m_onDisconnectedSlot);
			NazaraSlot(ServerConnection, OnRegisterFailure, m_onRegisterFailureSlot);
			NazaraSlot(ServerConnection, OnRegisterSuccess, m_onRegisterSuccess);

			StateData& m_stateData;
			Ndk::ButtonWidget* m_registerButton;
			Ndk::ButtonWidget* m_cancelButton;
			Ndk::LabelWidget* m_emailLabel;
			Ndk::LabelWidget* m_loginLabel;
			Ndk::LabelWidget* m_passwordLabel;
			Ndk::LabelWidget* m_passwordCheckLabel;
			Ndk::LabelWidget* m_statusLabel;
			Ndk::LabelWidget* m_titleLabel;
			Ndk::TextAreaWidget* m_emailArea;
			Ndk::TextAreaWidget* m_loginArea;
			Ndk::TextAreaWidget* m_passwordArea;
			Ndk::TextAreaWidget* m_passwordCheckArea;
			bool m_finished;
			bool m_isRegistering;
			float m_waitTime;
	};
}

#include <Client/States/RegisterState.inl>

#endif // EREWHON_CLIENT_STATES_REGISTERSTATE_HPP
