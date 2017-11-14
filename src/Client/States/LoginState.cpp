// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/LoginState.hpp>
#include <Nazara/Core/String.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/StateMachine.hpp>
#include <NDK/Widgets/CheckboxWidget.hpp>
#include <NDK/Widgets/LabelWidget.hpp>
#include <NDK/Widgets/TextAreaWidget.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Client/States/GameState.hpp>
#include <cassert>

namespace ewn
{
	void LoginState::Enter(Ndk::StateMachine& /*fsm*/)
	{
		m_loginSucceeded = false;

		m_statusLabel = m_stateData.canvas->Add<Ndk::LabelWidget>();
		m_statusLabel->UpdateText(Nz::SimpleTextDrawer::Draw("An error occured", 24));
		m_statusLabel->ResizeToContent();
		m_statusLabel->Show(false);

		m_loginLabel = m_stateData.canvas->Add<Ndk::LabelWidget>();
		m_loginLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Login: ", 24));
		m_loginLabel->ResizeToContent();

		m_loginArea = m_stateData.canvas->Add<Ndk::TextAreaWidget>();
		m_loginArea->EnableBackground(true);
		m_loginArea->SetBackgroundColor(Nz::Color::White);
		m_loginArea->SetSize({ 200.f, 36.f });
		m_loginArea->SetTextColor(Nz::Color::Black);

		m_passwordLabel = m_stateData.canvas->Add<Ndk::LabelWidget>();
		m_passwordLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Password: ", 24));
		m_passwordLabel->ResizeToContent();

		m_passwordArea = m_stateData.canvas->Add<Ndk::TextAreaWidget>();
		m_passwordArea->EnableBackground(true);
		m_passwordArea->SetBackgroundColor(Nz::Color::White);
		m_passwordArea->SetEchoMode(Ndk::EchoMode_Password);
		m_passwordArea->SetSize({ 200.f, 36.f });
		m_passwordArea->SetTextColor(Nz::Color::Black);

		m_connectionButton = m_stateData.canvas->Add<Ndk::ButtonWidget>();
		m_connectionButton->UpdateText(Nz::SimpleTextDrawer::Draw("Connection", 24));
		m_connectionButton->ResizeToContent();
		m_connectionButton->SetSize(m_connectionButton->GetSize() + Nz::Vector2f(10.f));
		m_connectionButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnConnectionPressed();
		});

		m_onLoginFailedSlot.Connect(m_stateData.app->OnLoginFailed, [this](const Packets::LoginFailure& loginFailure)
		{
			m_statusLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Login failed: " + std::to_string(loginFailure.reason), 24, 0L, Nz::Color::Red));
			m_statusLabel->CenterHorizontal();
			m_statusLabel->Show(true);
		});

		m_onLoginSucceededSlot.Connect(m_stateData.app->OnLoginSucceeded, [this](const Packets::LoginSuccess&)
		{
			m_statusLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Login succeeded", 24, 0L, Nz::Color::Green));
			m_statusLabel->CenterHorizontal();
			m_statusLabel->Show(true);

			m_loginSucceeded = true;
			m_loginAccumulator = 0.f;
		});

		LayoutWidgets();
	}

	void LoginState::Leave(Ndk::StateMachine& /*fsm*/)
	{
		m_connectionButton->Destroy();
		m_loginLabel->Destroy();
		m_loginArea->Destroy();
		m_passwordLabel->Destroy();
		m_passwordArea->Destroy();
		m_statusLabel->Destroy();
		m_onLoginFailedSlot.Disconnect();
		m_onLoginSucceededSlot.Disconnect();
	}

	bool LoginState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (m_loginSucceeded)
		{
			m_loginAccumulator += elapsedTime;
			if (m_loginAccumulator > 1.f)
				fsm.SetState(std::make_shared<GameState>(m_stateData));
		}

		return true;
	}

	void LoginState::OnConnectionPressed()
	{
		Packets::Login loginPacket;
		loginPacket.login = m_loginArea->GetText();
		loginPacket.passwordHash = ComputeHash(Nz::HashType_SHA256, m_passwordArea->GetText()).ToHex();

		m_stateData.app->SendPacket(loginPacket);
	}

	void LoginState::LayoutWidgets()
	{
		Nz::Vector2f center = m_stateData.canvas->GetSize() / 2.f;

		constexpr float padding = 10.f;

		float totalSize = m_statusLabel->GetSize().y + m_loginArea->GetSize().y + m_passwordArea->GetSize().y + m_connectionButton->GetSize().y + padding * 3.f;

		Nz::Vector2f cursor = center;
		cursor.y -= totalSize / 2.f;

		m_statusLabel->SetPosition({ 0.f, cursor.y, 0.f });
		m_statusLabel->CenterHorizontal();
		cursor.y += m_statusLabel->GetSize().y + padding;

		m_loginArea->SetPosition({ 0.f, cursor.y, 0.f });
		m_loginArea->CenterHorizontal();
		cursor.y += m_loginArea->GetSize().y + padding;

		m_loginLabel->SetPosition(m_loginArea->GetPosition() - Nz::Vector2f(m_loginLabel->GetSize().x, 0.f));

		m_passwordArea->SetPosition({ 0.f, cursor.y, 0.f });
		m_passwordArea->CenterHorizontal();
		cursor.y += m_passwordArea->GetSize().y + padding;

		m_passwordLabel->SetPosition(m_passwordArea->GetPosition() - Nz::Vector2f(m_passwordLabel->GetSize().x, 0.f));

		m_connectionButton->SetPosition({ 0.f, cursor.y, 0.f });
		m_connectionButton->CenterHorizontal();
	}
}
