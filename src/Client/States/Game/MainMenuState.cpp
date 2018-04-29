// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/MainMenuState.hpp>
#include <Nazara/Core/String.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/StateMachine.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/ConnectionLostState.hpp>
#include <Client/States/DisconnectionState.hpp>
#include <Client/States/Game/SpaceshipListState.hpp>
#include <Client/States/Game/TimeSyncState.hpp>
#include <cassert>

namespace ewn
{
	void MainMenuState::Enter(Ndk::StateMachine& /*fsm*/)
	{
		StateData& stateData = GetStateData();

		m_nextState.reset();

		m_disconnectButton = CreateWidget<Ndk::ButtonWidget>();
		m_disconnectButton->UpdateText(Nz::SimpleTextDrawer::Draw("Disconnect", 24));
		m_disconnectButton->SetPadding(10.f, 10.f, 10.f, 10.f);
		m_disconnectButton->ResizeToContent();
		m_disconnectButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnDisconnectPressed();
		});

		m_playButton = CreateWidget<Ndk::ButtonWidget>();
		m_playButton->UpdateText(Nz::SimpleTextDrawer::Draw("Play", 36));
		m_playButton->SetPadding(40.f, 10.f, 40.f, 10.f);
		m_playButton->ResizeToContent();
		m_playButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnPlayPressed();
		});

		m_spaceshipButton = CreateWidget<Ndk::ButtonWidget>();
		m_spaceshipButton->UpdateText(Nz::SimpleTextDrawer::Draw("Spaceship factory", 24));
		m_spaceshipButton->SetPadding(10.f, 10.f, 10.f, 10.f);
		m_spaceshipButton->ResizeToContent();
		m_spaceshipButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnSpaceshipFactoryPressed();
		});

		m_welcomeTextLabel = CreateWidget<Ndk::LabelWidget>();
		m_welcomeTextLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Welcome " + m_playerName, 24));
		m_welcomeTextLabel->ResizeToContent();

		LayoutWidgets();
		m_onTargetChangeSizeSlot.Connect(stateData.window->OnRenderTargetSizeChange, [this](const Nz::RenderTarget*) { LayoutWidgets(); });
	}

	void MainMenuState::Leave(Ndk::StateMachine& fsm)
	{
		AbstractState::Leave(fsm);

		m_onTargetChangeSizeSlot.Disconnect();
	}

	bool MainMenuState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		StateData& stateData = GetStateData();

		if (!stateData.server->IsConnected())
		{
			fsm.ChangeState(std::make_shared<ConnectionLostState>(stateData));
			return false;
		}

		if (m_nextState)
			fsm.ChangeState(m_nextState);

		return true;
	}

	void MainMenuState::LayoutWidgets()
	{
		Nz::Vector2f canvasSize = GetStateData().canvas->GetSize();

		m_welcomeTextLabel->CenterHorizontal();
		m_welcomeTextLabel->SetPosition({ m_welcomeTextLabel->GetPosition().x, m_welcomeTextLabel->GetSize().y * 2.f, 0.f });

		m_disconnectButton->CenterHorizontal();
		m_disconnectButton->SetPosition({ m_disconnectButton->GetPosition().x, canvasSize.y - m_disconnectButton->GetSize().y * 2.f, 0.f });

		m_playButton->CenterHorizontal();
		m_playButton->SetPosition({ m_playButton->GetPosition().x, m_disconnectButton->GetPosition().y - m_playButton->GetSize().y - 10.f, 0.f });
	
		m_spaceshipButton->SetPosition({ canvasSize.x / 6.f - m_spaceshipButton->GetSize().x / 2.f, canvasSize.y / 4.f - m_spaceshipButton->GetSize().y / 2.f, 0.f });
	}

	void MainMenuState::OnDisconnectPressed()
	{
		m_nextState = std::make_shared<DisconnectionState>(GetStateData(), false);
	}

	void MainMenuState::OnPlayPressed()
	{
		m_nextState = std::make_shared<TimeSyncState>(GetStateData());
	}

	void MainMenuState::OnSpaceshipFactoryPressed()
	{
		m_nextState = std::make_shared<SpaceshipListState>(GetStateData(), shared_from_this());
	}
}

