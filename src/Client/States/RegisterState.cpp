// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/RegisterState.hpp>
#include <Nazara/Core/String.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/StateMachine.hpp>
#include <NDK/Widgets/CheckboxWidget.hpp>
#include <NDK/Widgets/LabelWidget.hpp>
#include <NDK/Widgets/TextAreaWidget.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Client/States/LoginState.hpp>
#include <cassert>

namespace ewn
{
	void RegisterState::Enter(Ndk::StateMachine& /*fsm*/)
	{
		m_isRegistering = false;
		m_finished = false;

		m_statusLabel = m_stateData.canvas->Add<Ndk::LabelWidget>();
		m_statusLabel->Show(false);

		m_loginLabel = m_stateData.canvas->Add<Ndk::LabelWidget>();
		m_loginLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Login: ", 24));
		m_loginLabel->ResizeToContent();

		m_loginArea = m_stateData.canvas->Add<Ndk::TextAreaWidget>();
		m_loginArea->EnableBackground(true);
		m_loginArea->SetBackgroundColor(Nz::Color::White);
		m_loginArea->SetSize({ 300.f, 36.f });
		m_loginArea->SetTextColor(Nz::Color::Black);

		m_emailLabel = m_stateData.canvas->Add<Ndk::LabelWidget>();
		m_emailLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Email: ", 24));
		m_emailLabel->ResizeToContent();

		m_emailArea = m_stateData.canvas->Add<Ndk::TextAreaWidget>();
		m_emailArea->EnableBackground(true);
		m_emailArea->SetBackgroundColor(Nz::Color::White);
		m_emailArea->SetSize({ 300.f, 36.f });
		m_emailArea->SetTextColor(Nz::Color::Black);

		m_passwordLabel = m_stateData.canvas->Add<Ndk::LabelWidget>();
		m_passwordLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Password: ", 24));
		m_passwordLabel->ResizeToContent();

		m_passwordArea = m_stateData.canvas->Add<Ndk::TextAreaWidget>();
		m_passwordArea->EnableBackground(true);
		m_passwordArea->SetBackgroundColor(Nz::Color::White);
		m_passwordArea->SetEchoMode(Ndk::EchoMode_Password);
		m_passwordArea->SetSize({ 300.f, 36.f });
		m_passwordArea->SetTextColor(Nz::Color::Black);

		m_passwordCheckLabel = m_stateData.canvas->Add<Ndk::LabelWidget>();
		m_passwordCheckLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Password check: ", 24));
		m_passwordCheckLabel->ResizeToContent();

		m_passwordCheckArea = m_stateData.canvas->Add<Ndk::TextAreaWidget>();
		m_passwordCheckArea->EnableBackground(true);
		m_passwordCheckArea->SetBackgroundColor(Nz::Color::White);
		m_passwordCheckArea->SetEchoMode(Ndk::EchoMode_Password);
		m_passwordCheckArea->SetSize({ 300.f, 36.f });
		m_passwordCheckArea->SetTextColor(Nz::Color::Black);

		m_registerButton = m_stateData.canvas->Add<Ndk::ButtonWidget>();
		m_registerButton->UpdateText(Nz::SimpleTextDrawer::Draw("Register", 24));
		m_registerButton->ResizeToContent();
		m_registerButton->SetSize(m_registerButton->GetSize() + Nz::Vector2f(10.f));
		m_registerButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnRegisterPressed();
		});

		m_cancelButton = m_stateData.canvas->Add<Ndk::ButtonWidget>();
		m_cancelButton->UpdateText(Nz::SimpleTextDrawer::Draw("Cancel", 24));
		m_cancelButton->ResizeToContent();
		m_cancelButton->SetSize(m_cancelButton->GetSize() + Nz::Vector2f(10.f));
		m_cancelButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnCancelPressed();
		});

		// Set both connection and register button of the same width
		constexpr float buttonPadding = 5.f;
		float regConnWidth = std::max(m_registerButton->GetSize().x, m_cancelButton->GetSize().x) + buttonPadding;
		m_registerButton->SetSize({ regConnWidth, m_registerButton->GetSize().y + buttonPadding });
		m_cancelButton->SetSize({ regConnWidth, m_cancelButton->GetSize().y + buttonPadding });


		LayoutWidgets();

		// Slots
		m_onConnectedSlot.Connect(m_stateData.server->OnConnected, this, &RegisterState::OnConnected);
		m_onDisconnectedSlot.Connect(m_stateData.server->OnDisconnected, this, &RegisterState::OnDisconnected);

		m_onRegisterFailureSlot.Connect(m_stateData.server->OnRegisterFailure, [this](ServerConnection* connection, const Packets::RegisterFailure& loginFailure)
		{
			UpdateStatus("Registration failed: " + std::to_string(loginFailure.reason), Nz::Color::Red);
		});

		m_onRegisterSuccess.Connect(m_stateData.server->OnRegisterSuccess, [this](ServerConnection* connection, const Packets::RegisterSuccess&)
		{
			UpdateStatus("Registration succeeded", Nz::Color::Green);

			m_finished = true;
			m_waitTime = 2.f;
		});
	}

	void RegisterState::Leave(Ndk::StateMachine& /*fsm*/)
	{
		m_cancelButton->Destroy();
		m_loginLabel->Destroy();
		m_loginArea->Destroy();
		m_emailLabel->Destroy();
		m_emailArea->Destroy();
		m_passwordLabel->Destroy();
		m_passwordArea->Destroy();
		m_passwordCheckLabel->Destroy();
		m_passwordCheckArea->Destroy();
		m_registerButton->Destroy();
		m_statusLabel->Destroy();
		m_onConnectedSlot.Disconnect();
		m_onDisconnectedSlot.Disconnect();
		m_onRegisterFailureSlot.Disconnect();
		m_onRegisterSuccess.Disconnect();
	}

	bool RegisterState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (m_finished)
		{
			m_waitTime -= elapsedTime;
			if (m_waitTime <= 0.f)
				fsm.ChangeState(std::make_shared<LoginState>(m_stateData));
		}

		return true;
	}

	void RegisterState::OnCancelPressed()
	{
		m_finished = true;
		m_waitTime = 0.f;
	}

	void RegisterState::OnConnected(ServerConnection* /*server*/, Nz::UInt32 /*data*/)
	{
		if (m_isRegistering)
		{
			m_isRegistering = false;

			SendRegisterPacket();
		}
	}

	void RegisterState::OnDisconnected(ServerConnection* /*server*/, Nz::UInt32 /*data*/)
	{
		m_isRegistering = false;

		UpdateStatus("Error: failed to connect to server", Nz::Color::Red);
	}

	void RegisterState::OnRegisterPressed()
	{
		Nz::String login = m_loginArea->GetText();
		if (login.IsEmpty())
		{
			UpdateStatus("Error: blank login", Nz::Color::Red);
			return;
		}

		if (login.GetSize() > 20)
		{
			UpdateStatus("Error: Login is too long", Nz::Color::Red);
			return;
		}

		Nz::String email = m_emailArea->GetText();
		if (email.IsEmpty())
		{
			UpdateStatus("Error: blank email", Nz::Color::Red);
			return;
		}

		if (email.GetSize() > 40)
		{
			UpdateStatus("Error: Email is too long (are you insane?)", Nz::Color::Red);
			return;
		}

		Nz::String password = m_passwordArea->GetText();
		if (password.IsEmpty())
		{
			UpdateStatus("Error: blank password", Nz::Color::Red);
			return;
		}

		Nz::String passwordCheck = m_passwordCheckArea->GetText();
		if (passwordCheck.IsEmpty())
		{
			UpdateStatus("Error: blank password check", Nz::Color::Red);
			return;
		}

		if (password != passwordCheck)
		{
			UpdateStatus("Error: password mismatch", Nz::Color::Red);
			return;
		}

		if (m_stateData.server->IsConnected())
			SendRegisterPacket();
		else if (!m_isRegistering)
		{
			m_isRegistering = true;

			// Connect to server
			if (m_stateData.server->Connect(m_stateData.app->GetConfig().GetStringOption("Server.Address")))
				UpdateStatus("Connecting...");
			else
				UpdateStatus("Error: failed to initiate connection to server", Nz::Color::Red);
		}
	}

	void RegisterState::LayoutWidgets()
	{
		Nz::Vector2f center = m_stateData.canvas->GetSize() / 2.f;

		constexpr float padding = 10.f;

		std::array<Ndk::BaseWidget*, 7> widgets = {
			m_statusLabel,
			m_loginArea,
			m_emailArea,
			m_passwordArea,
			m_passwordCheckArea,
			m_registerButton,
			m_cancelButton
		};

		float totalSize = padding * (widgets.size() - 1);
		for (Ndk::BaseWidget* widget : widgets)
			totalSize += widget->GetSize().y;

		Nz::Vector2f cursor = center;
		cursor.y -= totalSize / 2.f;

		m_statusLabel->SetPosition({ 0.f, cursor.y, 0.f });
		m_statusLabel->CenterHorizontal();
		cursor.y += m_statusLabel->GetSize().y + padding;

		// Login
		m_loginArea->SetPosition({ 0.f, cursor.y, 0.f });
		m_loginArea->CenterHorizontal();
		cursor.y += m_loginArea->GetSize().y + padding;

		m_loginLabel->SetPosition(m_loginArea->GetPosition() - Nz::Vector2f(m_loginLabel->GetSize().x, 0.f));

		// Email
		m_emailArea->SetPosition({ 0.f, cursor.y, 0.f });
		m_emailArea->CenterHorizontal();
		cursor.y += m_emailArea->GetSize().y + padding;

		m_emailLabel->SetPosition(m_emailArea->GetPosition() - Nz::Vector2f(m_emailLabel->GetSize().x, 0.f));

		// Password
		m_passwordArea->SetPosition({ 0.f, cursor.y, 0.f });
		m_passwordArea->CenterHorizontal();
		cursor.y += m_passwordArea->GetSize().y + padding;

		m_passwordLabel->SetPosition(m_passwordArea->GetPosition() - Nz::Vector2f(m_passwordLabel->GetSize().x, 0.f));

		// Password check
		m_passwordCheckArea->SetPosition({ 0.f, cursor.y, 0.f });
		m_passwordCheckArea->CenterHorizontal();
		cursor.y += m_passwordCheckArea->GetSize().y + padding;

		m_passwordCheckLabel->SetPosition(m_passwordCheckArea->GetPosition() - Nz::Vector2f(m_passwordCheckLabel->GetSize().x, 0.f));

		m_registerButton->SetPosition({ 0.f, cursor.y, 0.f });
		m_registerButton->CenterHorizontal();
		cursor.y += m_registerButton->GetSize().y + padding;

		m_cancelButton->SetPosition({ 0.f, cursor.y, 0.f });
		m_cancelButton->CenterHorizontal();
		cursor.y += m_cancelButton->GetSize().y + padding;
	}

	void RegisterState::SendRegisterPacket()
	{
		Packets::Register registerPacket;
		registerPacket.login = m_loginArea->GetText().ToStdString();
		registerPacket.email = m_emailArea->GetText().ToStdString();

		// Salt password before hashing it
		Nz::String saltedPassword = m_loginArea->GetText().ToLower() + m_passwordArea->GetText() + "utopia";

		registerPacket.passwordHash = ComputeHash(Nz::HashType_SHA256, saltedPassword).ToHex().ToStdString();

		m_stateData.server->SendPacket(registerPacket);
	}

	void RegisterState::UpdateStatus(const Nz::String& status, const Nz::Color& color)
	{
		m_statusLabel->UpdateText(Nz::SimpleTextDrawer::Draw(status, 24, 0L, color));
		m_statusLabel->ResizeToContent();
		m_statusLabel->CenterHorizontal();
		m_statusLabel->Show(true);
	}
}

