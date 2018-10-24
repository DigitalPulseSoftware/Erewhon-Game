// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/EscapeMenuState.hpp>
#include <NDK/StateMachine.hpp>
#include <Client/ClientApplication.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/ConnectedState.hpp>
#include <Client/States/DisconnectionState.hpp>
#include <Client/States/OptionsState.hpp>
#include <Client/States/Game/MainMenuState.hpp>
#include <iostream>

namespace ewn
{
	void EscapeMenuState::Enter(Ndk::StateMachine& fsm)
	{
		AbstractState::Enter(fsm);

		StateData& stateData = GetStateData();

		m_disconnectButton = CreateWidget<Ndk::ButtonWidget>();
		m_disconnectButton->UpdateText(Nz::SimpleTextDrawer::Draw("Disconnect", 24));
		//m_disconnectButton->ResizeToContent();
		//m_disconnectButton->SetPadding(25.f, 25.f, 25.f, 25.f);
		m_disconnectButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			StateData& stateData = GetStateData();
			stateData.fsm->ResetState(std::make_shared<BackgroundState>(stateData));
			stateData.fsm->PushState(std::make_shared<DisconnectionState>(stateData, false));
		});

		m_leaveButton = CreateWidget<Ndk::ButtonWidget>();
		m_leaveButton->UpdateText(Nz::SimpleTextDrawer::Draw("Leave", 24));
		//m_leaveButton->ResizeToContent();
		//m_leaveButton->SetPadding(25.f, 25.f, 25.f, 25.f);
		m_leaveButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			StateData& stateData = GetStateData();
			stateData.fsm->ResetState(std::make_shared<BackgroundState>(stateData));
			stateData.fsm->PushState(std::make_shared<ConnectedState>(stateData));
			stateData.fsm->PushState(std::make_shared<MainMenuState>(stateData, "*TODO*"));
		});

		m_optionsButton = CreateWidget<Ndk::ButtonWidget>();
		m_optionsButton->UpdateText(Nz::SimpleTextDrawer::Draw("Options", 24));
		//m_optionsButton->ResizeToContent();
		//m_optionsButton->SetPadding(25.f, 25.f, 25.f, 25.f);
		m_optionsButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			StateData& stateData = GetStateData();
			stateData.fsm->ChangeState(std::make_shared<OptionsState>(stateData, shared_from_this()));
		});

		m_quitButton = CreateWidget<Ndk::ButtonWidget>();
		m_quitButton->UpdateText(Nz::SimpleTextDrawer::Draw("Quit", 24));
		//m_quitButton->ResizeToContent();
		//m_quitButton->SetPadding(25.f, 25.f, 25.f, 25.f);
		m_quitButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			StateData& stateData = GetStateData();
			stateData.fsm->ResetState(std::make_shared<BackgroundState>(stateData));
			stateData.fsm->PushState(std::make_shared<DisconnectionState>(stateData, true));
		});

		// Set all buttons to the same width
		float regConnWidth = std::max({ m_disconnectButton->GetSize().x, m_leaveButton->GetSize().x, m_optionsButton->GetSize().x, m_quitButton->GetSize().x });
		m_disconnectButton->Resize({ regConnWidth, m_disconnectButton->GetSize().y });
		m_leaveButton->Resize({ regConnWidth, m_leaveButton->GetSize().y });
		m_optionsButton->Resize({ regConnWidth, m_optionsButton->GetSize().y });
		m_quitButton->Resize({ regConnWidth, m_optionsButton->GetSize().y });

		LayoutWidgets();

		ConnectSignal(stateData.window->GetEventHandler().OnKeyPressed, this, &EscapeMenuState::OnKeyPressed);
	}

	void EscapeMenuState::LayoutWidgets()
	{
		Nz::Vector2f center = GetStateData().canvas->GetSize() / 2.f;

		constexpr float padding = 10.f;

		std::array<Ndk::BaseWidget*, 4> widgets = {
			m_optionsButton,
			m_leaveButton,
			m_disconnectButton,
			m_quitButton
		};

		float totalSize = padding * (widgets.size() - 1);
		for (Ndk::BaseWidget* widget : widgets)
		{
			Nz::Vector2f size = widget->GetSize();
			totalSize += size.y;
		}

		Nz::Vector2f cursor = center;
		cursor.y -= totalSize / 2.f;

		for (Ndk::BaseWidget* widget : widgets)
		{
			widget->SetPosition({ center.x - widget->GetSize().x / 2.f , cursor.y, 0.f });
			cursor.y += widget->GetSize().y + padding;
		}
	}

	void EscapeMenuState::OnKeyPressed(const Nz::EventHandler* /*eventHandler*/, const Nz::WindowEvent::KeyEvent& event)
	{
		if (event.code == Nz::Keyboard::Escape)
		{
			StateData& stateData = GetStateData();
			stateData.fsm->PopState();
		}
	}
}
