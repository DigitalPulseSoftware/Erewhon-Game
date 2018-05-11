// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_REGISTERSTATE_HPP
#define EREWHON_CLIENT_STATES_REGISTERSTATE_HPP

#include <Client/States/AbstractState.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>
#include <future>

namespace ewn
{
	class RegisterState final : public AbstractState
	{
		public:
			using AbstractState::AbstractState;
			~RegisterState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets() override;

			void OnCancelPressed();
			void OnConnected(ServerConnection* server, Nz::UInt32 data);
			void OnDisconnected(ServerConnection* server, Nz::UInt32 data);
			void OnRegisterPressed();

			void ComputePassword();
			void SendRegisterPacket();

			void UpdateStatus(const Nz::String& status, const Nz::Color& color = Nz::Color::White);

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
			std::future<std::string> m_passwordFuture;
			bool m_finished;
			bool m_isRegistering;
			float m_waitTime;
	};
}

#include <Client/States/RegisterState.inl>

#endif // EREWHON_CLIENT_STATES_REGISTERSTATE_HPP
