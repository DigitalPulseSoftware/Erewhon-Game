// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/EscapeMenuState.hpp>
#include <NDK/StateMachine.hpp>
#include <Client/ClientApplication.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/DisconnectionState.hpp>
#include <Client/States/OptionsState.hpp>
#include <iostream>

namespace ewn
{
	void EscapeMenuState::Enter(Ndk::StateMachine& /*fsm*/)
	{
		StateData& stateData = GetStateData();

		m_isDisconnecting = false;
		m_isLeavingMenu = false;
		m_isUsingOption = false;

		m_disconnectButton = CreateWidget<Ndk::ButtonWidget>();
		m_disconnectButton->UpdateText(Nz::SimpleTextDrawer::Draw("Disconnect", 24));
		m_disconnectButton->ResizeToContent();
		m_disconnectButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnDisconnectionPressed();
		});

		m_optionsButton = CreateWidget<Ndk::ButtonWidget>();
		m_optionsButton->UpdateText(Nz::SimpleTextDrawer::Draw("Options", 24));
		m_optionsButton->ResizeToContent();
		m_optionsButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnOptionsPressed();
		});

		// Set both connection and register button of the same width
		constexpr float buttonPadding = 25.f;
		float regConnWidth = std::max(m_disconnectButton->GetSize().x, m_optionsButton->GetSize().x) + buttonPadding;
		m_disconnectButton->SetSize({ regConnWidth, m_disconnectButton->GetSize().y + buttonPadding });
		m_optionsButton->SetSize({ regConnWidth, m_optionsButton->GetSize().y + buttonPadding });

		LayoutWidgets();
		m_onKeyPressedSlot.Connect(stateData.window->GetEventHandler().OnKeyPressed, this, &EscapeMenuState::OnKeyPressed);
		m_onTargetChangeSizeSlot.Connect(stateData.window->OnRenderTargetSizeChange, [this](const Nz::RenderTarget*) { LayoutWidgets(); });
	}

	bool EscapeMenuState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		StateData& stateData = GetStateData();

		if (m_isDisconnecting)
		{
			fsm.ResetState(std::make_shared<BackgroundState>(stateData));
			fsm.PushState(std::make_shared<DisconnectionState>(stateData));
		}
		else if (m_isLeavingMenu)
			fsm.PopState();
		else if (m_isUsingOption)
			fsm.ChangeState(std::make_shared<OptionsState>(stateData, shared_from_this()));

		return true;
	}

	void EscapeMenuState::LayoutWidgets()
	{
		Nz::Vector2f center = GetStateData().canvas->GetSize() / 2.f;

		constexpr float padding = 10.f;

		std::array<Ndk::BaseWidget*, 2> widgets = {
			m_disconnectButton,
			m_optionsButton
		};

		float maxWidth = 0.f;
		float totalSize = padding * (widgets.size() - 1);
		for (Ndk::BaseWidget* widget : widgets)
		{
			Nz::Vector2f size = widget->GetSize();
			maxWidth = std::max(maxWidth, size.x);
			totalSize += size.y;
		}

		Nz::Vector2f cursor = center;
		cursor.y -= totalSize / 2.f;

		m_optionsButton->SetPosition({ center.x - maxWidth / 2.f , cursor.y, 0.f });
		cursor.y += m_optionsButton->GetSize().y + padding;

		m_disconnectButton->SetPosition({ center.x - maxWidth / 2.f , cursor.y, 0.f });
		cursor.y += m_disconnectButton->GetSize().y + padding;
	}

	void EscapeMenuState::OnDisconnectionPressed()
	{
		m_isDisconnecting = true;
	}

	void EscapeMenuState::OnKeyPressed(const Nz::EventHandler* /*eventHandler*/, const Nz::WindowEvent::KeyEvent& event)
	{
		if (event.code == Nz::Keyboard::Escape)
			m_isLeavingMenu = true;
	}

	void EscapeMenuState::OnOptionsPressed()
	{
		m_isUsingOption = true;
	}
}
