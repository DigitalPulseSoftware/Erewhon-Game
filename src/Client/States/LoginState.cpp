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
#include <Client/States/RegisterState.hpp>
#include <Client/States/TimeSyncState.hpp>
#include <argon2/argon2.h>
#include <cassert>
#include <chrono>

namespace ewn
{
	void LoginState::Enter(Ndk::StateMachine& /*fsm*/)
	{
		m_isLoggingIn = false;
		m_loginSucceeded = false;
		m_isRegistering = false;

		m_statusLabel = m_stateData.canvas->Add<Ndk::LabelWidget>();
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

		m_rememberCheckbox = m_stateData.canvas->Add<Ndk::CheckboxWidget>();
		m_rememberCheckbox->UpdateText(Nz::SimpleTextDrawer::Draw("Remember me", 24));
		m_rememberCheckbox->ResizeToContent();

		m_onConnectedSlot.Connect(m_stateData.server->OnConnected, this, &LoginState::OnConnected);
		m_onDisconnectedSlot.Connect(m_stateData.server->OnDisconnected, this, &LoginState::OnDisconnected);

		m_connectionButton = m_stateData.canvas->Add<Ndk::ButtonWidget>();
		m_connectionButton->UpdateText(Nz::SimpleTextDrawer::Draw("Connection", 24));
		m_connectionButton->ResizeToContent();
		m_connectionButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnConnectionPressed();
		});

		m_registerButton = m_stateData.canvas->Add<Ndk::ButtonWidget>();
		m_registerButton->UpdateText(Nz::SimpleTextDrawer::Draw("Register", 24));
		m_registerButton->ResizeToContent();
		m_registerButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnRegisterPressed();
		});

		// Set both connection and register button of the same width
		constexpr float buttonPadding = 10.f;
		float regConnWidth = std::max(m_connectionButton->GetSize().x, m_registerButton->GetSize().x) + buttonPadding;
		m_connectionButton->SetSize({ regConnWidth, m_connectionButton->GetSize().y + buttonPadding });
		m_registerButton->SetSize({ regConnWidth, m_registerButton->GetSize().y + buttonPadding });

		m_onLoginFailureSlot.Connect(m_stateData.server->OnLoginFailure, [this](ServerConnection* connection, const Packets::LoginFailure& loginFailure)
		{
			std::string reason;
			switch (loginFailure.reason)
			{
				case LoginFailureReason::AccountNotFound:
					reason = "account not found";
					break;

				case LoginFailureReason::PasswordMismatch:
					reason = "password mismatch";
					break;

				case LoginFailureReason::ServerError:
					reason = "server error, please try again later";
					break;

				default:
					reason = "<packet error>";
					break;
			}

			UpdateStatus("Login failed: " + reason, Nz::Color::Red);
			m_isLoggingIn = false;
		});

		m_onLoginSuccess.Connect(m_stateData.server->OnLoginSuccess, [this](ServerConnection* connection, const Packets::LoginSuccess&)
		{
			UpdateStatus("Login succeeded", Nz::Color::Green);

			m_loginSucceeded = true;
			m_loginAccumulator = 0.f;
		});

		LayoutWidgets();

		// Fill with data from lastlogin.rememberme if present
		Nz::File loginFile("lastlogin.rememberme");
		if (loginFile.Open(Nz::OpenMode_ReadOnly))
		{
			Nz::String login = loginFile.ReadLine();
			Nz::String pass = loginFile.ReadLine();

			m_loginArea->SetText(login);
			m_passwordArea->SetText(pass);
			m_rememberCheckbox->SetState(Ndk::CheckboxState_Checked);
		}
	}

	void LoginState::Leave(Ndk::StateMachine& /*fsm*/)
	{
		m_connectionButton->Destroy();
		m_loginLabel->Destroy();
		m_loginArea->Destroy();
		m_passwordLabel->Destroy();
		m_passwordArea->Destroy();
		m_rememberCheckbox->Destroy();
		m_registerButton->Destroy();
		m_statusLabel->Destroy();
		m_onConnectedSlot.Disconnect();
		m_onDisconnectedSlot.Disconnect();
		m_onLoginFailureSlot.Disconnect();
		m_onLoginSuccess.Disconnect();
	}

	bool LoginState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (m_loginSucceeded)
		{
			m_loginAccumulator += elapsedTime;
			if (m_loginAccumulator > 1.f)
				fsm.ChangeState(std::make_shared<TimeSyncState>(m_stateData));
		}
		else if (m_isRegistering)
			fsm.ChangeState(std::make_shared<RegisterState>(m_stateData));
		else if (m_isLoggingIn)
		{
			// Computing password, wait for it
			if (m_passwordFuture.valid() && m_passwordFuture.wait_for(std::chrono::milliseconds(5)) == std::future_status::ready)
			{
				if (m_stateData.server->IsConnected())
					SendLoginPacket();
			}
		}

		return true;
	}

	void LoginState::OnConnectionPressed()
	{
		if (m_isLoggingIn)
			return;

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

		Nz::String password = m_passwordArea->GetText();
		if (password.GetLength() < 8)
		{
			UpdateStatus("Error: password is too short (at least 8 characters required)", Nz::Color::Red);
			return;
		}

		Nz::File loginFile("lastlogin.rememberme");
		if (m_rememberCheckbox->GetState() == Ndk::CheckboxState_Checked)
		{
			if (loginFile.Open(Nz::OpenMode_Truncate | Nz::OpenMode_WriteOnly))
				loginFile.Write(m_loginArea->GetText() + '\n' + m_passwordArea->GetText());
			else
				std::cerr << "Failed to open remember.me file" << std::endl;
		}
		else if (loginFile.Exists())
			loginFile.Delete();

		ComputePassword();

		if (!m_stateData.server->IsConnected())
		{
			m_isLoggingIn = true;

			// Connect to server
			if (m_stateData.server->Connect(m_stateData.app->GetConfig().GetStringOption("Server.Address")))
			{
				UpdateStatus("Connecting...");
				m_isLoggingIn = true;
			}
			else
				UpdateStatus("Error: failed to initiate connection to server", Nz::Color::Red);
		}
		else
		{
			UpdateStatus("Logging in...");
			m_isLoggingIn = true;
		}
	}

	void LoginState::OnConnected(ServerConnection* /*server*/, Nz::UInt32 /*data*/)
	{
		if (m_isLoggingIn)
			UpdateStatus("Logging in...");
	}

	void LoginState::OnDisconnected(ServerConnection* /*server*/, Nz::UInt32 /*data*/)
	{
		m_isLoggingIn = false;

		UpdateStatus("Error: failed to connect to server", Nz::Color::Red);
	}

	void LoginState::OnRegisterPressed()
	{
		if (m_isLoggingIn)
			return;

		m_isRegistering = true;
	}

	void LoginState::LayoutWidgets()
	{
		Nz::Vector2f center = m_stateData.canvas->GetSize() / 2.f;

		constexpr float padding = 10.f;

		std::array<Ndk::BaseWidget*, 5> widgets = {
			m_statusLabel,
			m_loginArea,
			m_passwordArea,
			m_connectionButton,
			m_registerButton
		};

		float totalSize = padding * (widgets.size() - 1);
		for (Ndk::BaseWidget* widget : widgets)
			totalSize += widget->GetSize().y;

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

		m_rememberCheckbox->SetPosition({ 0.f, cursor.y, 0.f });
		m_rememberCheckbox->CenterHorizontal();
		cursor.y += m_rememberCheckbox->GetSize().y + padding;

		m_connectionButton->SetPosition({ 0.f, cursor.y, 0.f });
		m_connectionButton->CenterHorizontal();
		cursor.y += m_connectionButton->GetSize().y + padding;

		m_registerButton->SetPosition({ 0.f, cursor.y, 0.f });
		m_registerButton->CenterHorizontal();
		cursor.y += m_registerButton->GetSize().y + padding;
	}

	void LoginState::ComputePassword()
	{
		// Salt password before hashing it
		const ConfigFile& config = m_stateData.app->GetConfig();

		int iCost = config.GetIntegerOption("Security.Argon2.IterationCost");
		int mCost = config.GetIntegerOption("Security.Argon2.MemoryCost");
		int tCost = config.GetIntegerOption("Security.Argon2.ThreadCost");
		int hashLength = config.GetIntegerOption("Security.HashLength");
		const std::string& salt = config.GetStringOption("Security.PasswordSalt");

		Nz::String saltedPassword = m_loginArea->GetText().ToLower() + m_passwordArea->GetText();

		m_passwordFuture = std::async(std::launch::async, [pwd = std::move(saltedPassword), &salt, iCost, mCost, tCost, hashLength]()->std::string
		{
			std::string hash(hashLength, '\0');
			if (argon2_hash(iCost, mCost, tCost, pwd.GetConstBuffer(), pwd.GetSize(), salt.data(), salt.size(), hash.data(), hash.size(), nullptr, 0, argon2_type::Argon2_id, ARGON2_VERSION_13) != ARGON2_OK)
				hash.clear();

			return hash;
		});
	}

	void LoginState::SendLoginPacket()
	{
		std::string hashedPassword = m_passwordFuture.get();
		if (hashedPassword.empty())
		{
			UpdateStatus("Failed to hash password", Nz::Color::Red);
			m_isLoggingIn = false;
			return;
		}

		Packets::Login loginPacket;
		loginPacket.login = m_loginArea->GetText().ToStdString();
		loginPacket.passwordHash = hashedPassword;

		m_stateData.server->SendPacket(loginPacket);
	}

	void LoginState::UpdateStatus(const Nz::String& status, const Nz::Color& color)
	{
		m_statusLabel->UpdateText(Nz::SimpleTextDrawer::Draw(status, 24, 0L, color));
		m_statusLabel->ResizeToContent();
		m_statusLabel->CenterHorizontal();
		m_statusLabel->Show(true);
	}
}
