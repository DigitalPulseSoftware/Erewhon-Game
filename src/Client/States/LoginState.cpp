// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/LoginState.hpp>
#include <Nazara/Core/String.hpp>
#include <Nazara/Renderer/DebugDrawer.hpp>
#include <Nazara/Renderer/Renderer.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/StateMachine.hpp>
#include <NDK/Widgets/CheckboxWidget.hpp>
#include <NDK/Widgets/LabelWidget.hpp>
#include <NDK/Widgets/TextAreaWidget.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Client/States/ConnectedState.hpp>
#include <Client/States/Game/MainMenuState.hpp>
#include <Client/States/OptionsState.hpp>
#include <Client/States/RegisterState.hpp>
#include <argon2/argon2.h>
#include <cassert>
#include <chrono>

namespace ewn
{
	static constexpr const char* TokenFile = "connectiontoken.rememberme";

	void LoginState::Enter(Ndk::StateMachine& fsm)
	{
		AbstractState::Enter(fsm);

		StateData& stateData = GetStateData();

		m_isLoggingIn = false;
		m_isLoggingInByToken = false;
		m_loginSucceeded = false;

		m_statusLabel = CreateWidget<Ndk::LabelWidget>();
		m_statusLabel->Show(false);

		m_loginLabel = CreateWidget<Ndk::LabelWidget>();
		m_loginLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Login: ", 24));
		m_loginLabel->ResizeToContent();

		m_loginArea = CreateWidget<Ndk::TextAreaWidget>();
		m_loginArea->EnableBackground(true);
		m_loginArea->SetBackgroundColor(Nz::Color::White);
		m_loginArea->SetSize({ 200.f, 36.f });
		m_loginArea->SetTextColor(Nz::Color::Black);

		m_passwordLabel = CreateWidget<Ndk::LabelWidget>();
		m_passwordLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Password: ", 24));
		m_passwordLabel->ResizeToContent();

		m_passwordArea = CreateWidget<Ndk::TextAreaWidget>();
		m_passwordArea->EnableBackground(true);
		m_passwordArea->SetBackgroundColor(Nz::Color::White);
		m_passwordArea->SetEchoMode(Ndk::EchoMode_Password);
		m_passwordArea->SetSize({ 200.f, 36.f });
		m_passwordArea->SetTextColor(Nz::Color::Black);

		m_rememberCheckbox = CreateWidget<Ndk::CheckboxWidget>();
		m_rememberCheckbox->UpdateText(Nz::SimpleTextDrawer::Draw("Remember me", 24));
		m_rememberCheckbox->ResizeToContent();

		m_connectionButton = CreateWidget<Ndk::ButtonWidget>();
		m_connectionButton->UpdateText(Nz::SimpleTextDrawer::Draw("Connection", 24));
		m_connectionButton->SetPadding(10.f, 10.f, 10.f, 10.f);
		m_connectionButton->ResizeToContent();
		m_connectionButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnConnectionPressed();
		});

		m_optionButton = CreateWidget<Ndk::ButtonWidget>();
		m_optionButton->UpdateText(Nz::SimpleTextDrawer::Draw("Options", 24));
		m_optionButton->SetPadding(10.f, 10.f, 10.f, 10.f);
		m_optionButton->ResizeToContent();
		m_optionButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnOptionPressed();
		});

		m_quitButton = CreateWidget<Ndk::ButtonWidget>();
		m_quitButton->UpdateText(Nz::SimpleTextDrawer::Draw("Quit", 24));
		m_quitButton->SetPadding(10.f, 10.f, 10.f, 10.f);
		m_quitButton->ResizeToContent();
		m_quitButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnQuitPressed();
		});

		m_registerButton = CreateWidget<Ndk::ButtonWidget>();
		m_registerButton->UpdateText(Nz::SimpleTextDrawer::Draw("Register", 24));
		m_registerButton->SetPadding(10.f, 10.f, 10.f, 10.f);
		m_registerButton->ResizeToContent();
		m_registerButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnRegisterPressed();
		});

		// Set both connection and register button of the same width
		float maxButtonWidth = std::max({ m_connectionButton->GetSize().x, m_registerButton->GetSize().x });
		m_connectionButton->SetSize({ maxButtonWidth, m_connectionButton->GetSize().y });
		m_registerButton->SetSize({ maxButtonWidth, m_registerButton->GetSize().y });

		LayoutWidgets();

		ConnectSignal(stateData.server->OnConnected, this, &LoginState::OnConnected);
		ConnectSignal(stateData.server->OnDisconnected, this, &LoginState::OnDisconnected);

		ConnectSignal(stateData.server->OnLoginFailure, [this](ServerConnection* connection, const Packets::LoginFailure& loginFailure)
		{
			std::string reason;
			switch (loginFailure.reason)
			{
				case LoginFailureReason::AccountNotFound:
					reason = "account not found";
					break;

				case LoginFailureReason::InvalidToken:
					reason = "automatic connection token expired";
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
			m_isLoggingInByToken = false;
		});

		ConnectSignal(stateData.server->OnLoginSuccess, [this](ServerConnection* connection, const Packets::LoginSuccess& loginPacket)
		{
			UpdateStatus("Login succeeded", Nz::Color::Green);

			m_loginSucceeded = true;
			m_loginAccumulator = 0.f;

			if (m_rememberCheckbox->GetState() == Ndk::CheckboxState_Checked && !loginPacket.connectionToken.empty())
			{
				Nz::File loginFile(TokenFile);
				if (loginFile.Open(Nz::OpenMode_Truncate | Nz::OpenMode_WriteOnly))
				{
					Nz::String login = m_loginArea->GetText();
					Nz::String tokenAsString(loginPacket.connectionToken.size() * 2, '\0');
					for (std::size_t i = 0; i < loginPacket.connectionToken.size(); ++i)
						std::sprintf(&tokenAsString[i * 2], "%02x", loginPacket.connectionToken[i]);

					loginFile.Write(login + '\n' + tokenAsString);
				}
				else
					std::cerr << "Failed to open remember me file" << std::endl;
			}
		});

		LoadTokenFile();
	}

	bool LoginState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		StateData& stateData = GetStateData();

		if (m_loginSucceeded)
		{
			m_loginAccumulator += elapsedTime;
			if (m_loginAccumulator > 1.f)
			{
				fsm.PopState();
				fsm.PushState(std::make_shared<ConnectedState>(stateData));
				fsm.PushState(std::make_shared<MainMenuState>(stateData, m_loginArea->GetText().ToStdString()));
			}
		}
		else if (m_isLoggingIn)
		{
			// Computing password, wait for it
			if (m_passwordFuture.valid() && m_passwordFuture.wait_for(std::chrono::milliseconds(5)) == std::future_status::ready)
			{
				if (stateData.server->IsConnected())
					SendLoginPacket();
			}
		}

		return true;
	}

	void LoginState::OnConnectionPressed()
	{
		StateData& stateData = GetStateData();

		if (m_isLoggingIn || m_isLoggingInByToken)
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
		if (password.IsEmpty() && !m_connectionToken.empty())
		{
			// Use login by token if password is empty
			if (m_rememberCheckbox->GetState() == Ndk::CheckboxState_Unchecked)
			{
				if (Nz::File::Exists(TokenFile))
					Nz::File::Delete(TokenFile);
			}

			if (!stateData.server->IsConnected())
			{
				// Connect to server
				if (stateData.server->Connect(stateData.app->GetConfig().GetStringOption("Server.Address")))
				{
					UpdateStatus("Connecting...");
					m_isLoggingInByToken = true;
				}
				else
					UpdateStatus("Error: failed to initiate connection to server", Nz::Color::Red);
			}
			else
			{
				UpdateStatus("Auto-logging in...");
				m_isLoggingInByToken = true;
				SendLoginByTokenPacket();
			}
		}
		else
		{
			if (password.GetLength() < 8)
			{
				UpdateStatus("Error: password is too short (at least 8 characters required)", Nz::Color::Red);
				return;
			}

			if (m_rememberCheckbox->GetState() == Ndk::CheckboxState_Unchecked)
				Nz::File::Delete(TokenFile);

			ComputePassword();

			if (!stateData.server->IsConnected())
			{
				// Connect to server
				if (stateData.server->Connect(stateData.app->GetConfig().GetStringOption("Server.Address")))
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
	}

	void LoginState::OnConnected(ServerConnection* server, Nz::UInt32 /*data*/)
	{
		if (m_isLoggingInByToken)
		{
			SendLoginByTokenPacket();
			UpdateStatus("Auto-logging in...");
		}
		else if (m_isLoggingIn)
			UpdateStatus("Logging in...");
	}

	void LoginState::OnDisconnected(ServerConnection* /*server*/, Nz::UInt32 /*data*/)
	{
		m_isLoggingIn = false;
		m_isLoggingInByToken = false;

		UpdateStatus("Error: failed to connect to server", Nz::Color::Red);
	}

	void LoginState::OnQuitPressed()
	{
		GetStateData().app->Quit();
	}

	void LoginState::OnOptionPressed()
	{
		StateData& stateData = GetStateData();
		stateData.fsm->ChangeState(std::make_shared<OptionsState>(stateData, shared_from_this()));
	}

	void LoginState::OnRegisterPressed()
	{
		if (m_isLoggingIn || m_isLoggingInByToken)
			return;

		StateData& stateData = GetStateData();
		stateData.fsm->ChangeState(std::make_shared<RegisterState>(stateData));
	}

	void LoginState::LayoutWidgets()
	{
		Nz::Vector2f canvasSize = GetStateData().canvas->GetSize();
		Nz::Vector2f center = canvasSize / 2.f;

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

		constexpr float optionButtonPadding = 20.f;
		m_optionButton->SetPosition(optionButtonPadding, canvasSize.y - m_optionButton->GetSize().y - optionButtonPadding);
		m_quitButton->SetPosition(canvasSize.x - m_quitButton->GetSize().x - optionButtonPadding, canvasSize.y - m_quitButton->GetSize().y - optionButtonPadding);
	}

	void LoginState::ComputePassword()
	{
		// Salt password before hashing it
		const ConfigFile& config = GetStateData().app->GetConfig();

		int iCost = config.GetIntegerOption<int>("Security.Argon2.IterationCost");
		int mCost = config.GetIntegerOption<int>("Security.Argon2.MemoryCost");
		int tCost = config.GetIntegerOption<int>("Security.Argon2.ThreadCost");
		int hashLength = config.GetIntegerOption<int>("Security.HashLength");
		const std::string& salt = config.GetStringOption("Security.PasswordSalt");

		Nz::String saltedPassword = m_loginArea->GetText().ToLower() + m_passwordArea->GetText();

		m_passwordFuture = std::async(std::launch::async, [pwd = std::move(saltedPassword), &salt, iCost, mCost, tCost, hashLength]() -> std::string
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
		loginPacket.generateConnectionToken = (m_rememberCheckbox->GetState() == Ndk::CheckboxState_Checked);
		loginPacket.login = m_loginArea->GetText().ToStdString();
		loginPacket.passwordHash = hashedPassword;

		GetStateData().server->SendPacket(loginPacket);
	}

	void LoginState::SendLoginByTokenPacket()
	{
		Packets::LoginByToken loginPacket;
		loginPacket.connectionToken = std::move(m_connectionToken);
		loginPacket.generateConnectionToken = true;

		GetStateData().server->SendPacket(loginPacket);

		m_connectionToken.clear(); //< Ensure state of connection token
	}

	void LoginState::UpdateStatus(const Nz::String& status, const Nz::Color& color)
	{
		m_statusLabel->UpdateText(Nz::SimpleTextDrawer::Draw(status, 24, 0L, color));
		m_statusLabel->ResizeToContent();
		m_statusLabel->CenterHorizontal();
		m_statusLabel->Show(true);
	}

	void LoginState::LoadTokenFile()
	{
		StateData& stateData = GetStateData();

		Nz::File loginFile(TokenFile);
		if (loginFile.Open(Nz::OpenMode_ReadOnly))
		{
			Nz::String login = loginFile.ReadLine();
			Nz::String token = loginFile.ReadLine();
			if (token.GetSize() == 128)
			{
				std::vector<Nz::UInt8> binToken;
				binToken.reserve(64);
				for (std::size_t i = 0; i < 128; i += 2)
				{
					static const char* hexadecimal = "0123456789abcdef";

					char c1 = token[i];
					char c2 = token[i + 1];

					const char* p1 = std::strchr(hexadecimal, c1);
					const char* p2 = std::strchr(hexadecimal, c2);
					if (!p1 || !p2)
						return;

					std::size_t v1 = p1 - hexadecimal;
					std::size_t v2 = p2 - hexadecimal;

					binToken.push_back((v1 * 16) + v2);
				}
				m_connectionToken = std::move(binToken);

				m_loginArea->SetText(login);
				m_rememberCheckbox->SetState(Ndk::CheckboxState_Checked);

				if (m_shouldAutoLogin)
				{
					if (!stateData.server->IsConnected())
					{
						// Connect to server
						if (stateData.server->Connect(stateData.app->GetConfig().GetStringOption("Server.Address")))
						{
							UpdateStatus("Connecting...");
							m_isLoggingInByToken = true;
						}
						else
							UpdateStatus("Error: failed to initiate connection to server", Nz::Color::Red);
					}
					else
					{
						UpdateStatus("Auto-logging in...");
						m_isLoggingInByToken = true;

						Packets::LoginByToken loginPacket;
						loginPacket.connectionToken = std::move(m_connectionToken);
						loginPacket.generateConnectionToken = true;

						stateData.server->SendPacket(loginPacket);
					}
				}
			}
		}
	}

}
