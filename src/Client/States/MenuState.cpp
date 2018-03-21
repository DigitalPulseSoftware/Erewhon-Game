// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/MenuState.hpp>
#include <NDK/StateMachine.hpp>
#include <Client/ClientApplication.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/DisconnectionState.hpp>
#include <iostream>

namespace ewn
{
	void MenuState::Enter(Ndk::StateMachine& /*fsm*/)
	{
		m_isDisconnecting = false;
		m_isLeavingMenu = false;

		m_disconnectButton = m_stateData.canvas->Add<Ndk::ButtonWidget>();
		m_disconnectButton->UpdateText(Nz::SimpleTextDrawer::Draw("Disconnect", 24));
		m_disconnectButton->ResizeToContent();
		m_disconnectButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnDisconnectionPressed();
		});

		m_optionsButton = m_stateData.canvas->Add<Ndk::ButtonWidget>();
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
		m_onKeyPressedSlot.Connect(m_stateData.window->GetEventHandler().OnKeyPressed, this, &MenuState::OnKeyPressed);
		m_onTargetChangeSizeSlot.Connect(m_stateData.window->OnRenderTargetSizeChange, [this](const Nz::RenderTarget*) { LayoutWidgets(); });
	}

	void MenuState::Leave(Ndk::StateMachine& /*fsm*/)
	{
		m_disconnectButton->Destroy();
		m_optionsButton->Destroy();
	}

	bool MenuState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (m_isDisconnecting)
		{
			fsm.ResetState(std::make_shared<BackgroundState>(m_stateData));
			fsm.PushState(std::make_shared<DisconnectionState>(m_stateData));
		}
		else if (m_isLeavingMenu)
			fsm.PopState();

		return true;
	}

	void MenuState::LayoutWidgets()
	{
		Nz::Vector2f center = m_stateData.canvas->GetSize() / 2.f;

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

	void MenuState::OnDisconnectionPressed()
	{
		m_isDisconnecting = true;
	}

	void MenuState::OnKeyPressed(const Nz::EventHandler* /*eventHandler*/, const Nz::WindowEvent::KeyEvent& event)
	{
		if (event.code == Nz::Keyboard::Escape)
			m_isLeavingMenu = true;
	}

	void MenuState::OnOptionsPressed()
	{
		std::cout << "Option menu is not implemented yet" << std::endl;
	}
}
