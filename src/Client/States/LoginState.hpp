// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_LOGINSTATE_HPP
#define EREWHON_CLIENT_STATES_LOGINSTATE_HPP

#include <Client/States/AbstractState.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>
#include <future>
#include <vector>

namespace ewn
{
	class LoginState final : public AbstractState
	{
		public:
			inline LoginState(StateData& stateData, bool shouldAutoLogin = false);
			~LoginState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;

			void LoadTokenFile();

			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets() override;

			void OnConnected(ServerConnection* server, Nz::UInt32 data);
			void OnConnectionPressed();
			void OnDisconnected(ServerConnection* server, Nz::UInt32 data);
			void OnQuitPressed();
			void OnOptionPressed();
			void OnRegisterPressed();

			void ComputePassword();
			void SendLoginPacket();
			void SendLoginByTokenPacket();

			void UpdateStatus(const Nz::String& status, const Nz::Color& color = Nz::Color::White);

			Ndk::ButtonWidget* m_connectionButton;
			Ndk::ButtonWidget* m_optionButton;
			Ndk::ButtonWidget* m_quitButton;
			Ndk::ButtonWidget* m_registerButton;
			Ndk::CheckboxWidget* m_rememberCheckbox;
			Ndk::LabelWidget* m_loginLabel;
			Ndk::LabelWidget* m_passwordLabel;
			Ndk::LabelWidget* m_statusLabel;
			Ndk::TextAreaWidget* m_loginArea;
			Ndk::TextAreaWidget* m_passwordArea;
			std::future<std::string> m_passwordFuture;
			std::vector<Nz::UInt8> m_connectionToken;
			bool m_isLoggingIn;
			bool m_isLoggingInByToken;
			bool m_loginSucceeded;
			bool m_shouldAutoLogin;
			float m_loginAccumulator;
	};
}

#include <Client/States/LoginState.inl>

#endif // EREWHON_CLIENT_STATES_LOGINSTATE_HPP
