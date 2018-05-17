// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
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
	void MainMenuState::Enter(Ndk::StateMachine& fsm)
	{
		AbstractState::Enter(fsm);

		StateData& stateData = GetStateData();

		m_disconnectButton = CreateWidget<Ndk::ButtonWidget>();
		m_disconnectButton->UpdateText(Nz::SimpleTextDrawer::Draw("Disconnect", 24));
		m_disconnectButton->SetPadding(10.f, 10.f, 10.f, 10.f);
		m_disconnectButton->ResizeToContent();
		m_disconnectButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnDisconnectPressed();
		});

		m_refreshButton = CreateWidget<Ndk::ButtonWidget>();
		m_refreshButton->UpdateText(Nz::SimpleTextDrawer::Draw("Refresh arenas", 24));
		m_refreshButton->SetPadding(10.f, 10.f, 10.f, 10.f);
		m_refreshButton->ResizeToContent();
		m_refreshButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnRefreshPressed();
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

		ConnectSignal(stateData.server->OnArenaList, this, &MainMenuState::OnArenaList);

		stateData.server->SendPacket(Packets::QueryArenaList{});
	}

	void MainMenuState::Leave(Ndk::StateMachine& fsm)
	{
		AbstractState::Leave(fsm);

		m_arenaButtons.clear();
	}

	void MainMenuState::LayoutWidgets()
	{
		Nz::Vector2f canvasSize = GetStateData().canvas->GetSize();

		// Left
		Nz::Vector2f leftCursor = { canvasSize.x / 6.f, canvasSize.y / 4.f };

		m_spaceshipButton->SetPosition({ leftCursor.x - m_spaceshipButton->GetSize().x / 2.f, leftCursor.y, 0.f });
		leftCursor.y += m_spaceshipButton->GetSize().y + 10.f;


		// Center
		m_welcomeTextLabel->CenterHorizontal();
		m_welcomeTextLabel->SetPosition({ m_welcomeTextLabel->GetPosition().x, m_welcomeTextLabel->GetSize().y * 2.f, 0.f });

		m_disconnectButton->CenterHorizontal();
		m_disconnectButton->SetPosition({ m_disconnectButton->GetPosition().x, canvasSize.y - m_disconnectButton->GetSize().y * 2.f, 0.f });

		m_refreshButton->CenterHorizontal();
		m_refreshButton->SetPosition({ m_refreshButton->GetPosition().x, m_disconnectButton->GetPosition().y - m_refreshButton->GetSize().y - 10.f, 0.f });
	
		static constexpr float modulePadding = 10.f;

		Nz::Vector2f cursor(canvasSize.x * 0.5f, canvasSize.y * 0.3f);
		for (Ndk::ButtonWidget* button : m_arenaButtons)
		{
			button->SetPosition({ cursor.x - button->GetSize().x / 2.f, cursor.y, 0.f });
			cursor.y += button->GetSize().y + modulePadding;
		}
	}

	void MainMenuState::OnArenaButtonPressed(std::size_t arenaId)
	{
		GetStateData().fsm->ChangeState(std::make_shared<TimeSyncState>(GetStateData(), static_cast<Nz::UInt8>(arenaId)));
	}

	void MainMenuState::OnArenaList(ServerConnection* server, const Packets::ArenaList& arenaList)
	{
		for (Ndk::ButtonWidget* button : m_arenaButtons)
			DestroyWidget(button);

		m_arenaButtons.clear();

		std::size_t arenaIndex = 0;
		for (const auto& arenaData : arenaList.arenas)
		{
			Ndk::ButtonWidget* button = CreateWidget<Ndk::ButtonWidget>();
			button->UpdateText(Nz::SimpleTextDrawer::Draw("Join arena #" + std::to_string(arenaIndex) + ": " + arenaData.arenaName, 24));
			button->SetPadding(10.f, 10.f, 10.f, 10.f);
			button->ResizeToContent();
			button->OnButtonTrigger.Connect([this, arenaIndex](const Ndk::ButtonWidget*)
			{
				OnArenaButtonPressed(arenaIndex);
			});

			m_arenaButtons.push_back(button);

			arenaIndex++;
		}

		LayoutWidgets();
	}

	void MainMenuState::OnDisconnectPressed()
	{
		StateData& stateData = GetStateData();
		stateData.fsm->ResetState(std::make_shared<BackgroundState>(stateData));
		stateData.fsm->PushState(std::make_shared<DisconnectionState>(stateData, false));
	}

	void MainMenuState::OnFleetManagementPressed()
	{
	}

	void MainMenuState::OnRefreshPressed()
	{
		GetStateData().server->SendPacket(Packets::QueryArenaList{});
	}

	void MainMenuState::OnSpaceshipFactoryPressed()
	{
		GetStateData().fsm->ChangeState(std::make_shared<SpaceshipListState>(GetStateData(), shared_from_this()));
	}
}

