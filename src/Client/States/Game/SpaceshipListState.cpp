// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/SpaceshipListState.hpp>
#include <Nazara/Core/String.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/StateMachine.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/ConnectionLostState.hpp>
#include <Client/States/Game/MainMenuState.hpp>
#include <Client/States/Game/SpaceshipEditState.hpp>
#include <Client/States/Game/TimeSyncState.hpp>
#include <cassert>

namespace ewn
{
	void SpaceshipListState::Enter(Ndk::StateMachine& /*fsm*/)
	{
		StateData& stateData = GetStateData();

		m_nextState.reset();

		m_statusLabel = CreateWidget<Ndk::LabelWidget>();

		m_titleLabel = CreateWidget<Ndk::LabelWidget>();
		m_titleLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Spaceships:", 24));
		m_titleLabel->ResizeToContent();

		m_backButton = CreateWidget<Ndk::ButtonWidget>();
		m_backButton->UpdateText(Nz::SimpleTextDrawer::Draw("Back to main menu", 24));
		m_backButton->SetPadding(10.f, 10.f, 10.f, 10.f);
		m_backButton->ResizeToContent();
		m_backButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnBackPressed();
		});

		LayoutWidgets();
		m_onTargetChangeSizeSlot.Connect(stateData.window->OnRenderTargetSizeChange, [this](const Nz::RenderTarget*) { LayoutWidgets(); });

		m_onSpaceshipListSlot.Connect(stateData.server->OnSpaceshipList, this, &SpaceshipListState::OnSpaceshipList);

		QuerySpaceships();
	}

	void SpaceshipListState::Leave(Ndk::StateMachine& fsm)
	{
		AbstractState::Leave(fsm);

		m_spaceshipButtons.clear();
		m_onSpaceshipListSlot.Disconnect();
		m_onTargetChangeSizeSlot.Disconnect();
	}

	bool SpaceshipListState::Update(Ndk::StateMachine& fsm, float elapsedTime)
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

	void SpaceshipListState::LayoutWidgets()
	{
		Nz::Vector2f canvasSize = GetStateData().canvas->GetSize();

		m_statusLabel->Center();

		m_backButton->CenterHorizontal();
		m_backButton->SetPosition({ m_backButton->GetPosition().x, canvasSize.y - m_backButton->GetSize().y * 2.f, 0.f });

		float totalHeight = 0.f;
		for (Ndk::BaseWidget* button : m_spaceshipButtons)
		{
			constexpr float margin = 20.f;

			totalHeight += button->GetSize().y + margin;
		}

		float buttonTop = canvasSize.y / 2.f - totalHeight / 2.f;

		m_titleLabel->CenterHorizontal();
		m_titleLabel->SetPosition(m_titleLabel->GetPosition().x, buttonTop - m_titleLabel->GetSize().y - 5.f);

		float buttonSpace = totalHeight / m_spaceshipButtons.size();
		for (std::size_t i = 0; i < m_spaceshipButtons.size(); ++i)
		{
			Ndk::BaseWidget* button = m_spaceshipButtons[i];
			button->CenterHorizontal();
			button->SetPosition(button->GetPosition().x, buttonTop + i * buttonSpace);
		}
	}

	void SpaceshipListState::QuerySpaceships()
{
		m_titleLabel->Show(false);

		UpdateStatus("Loading spaceships...");
		GetStateData().server->SendPacket(Packets::QuerySpaceshipList());
	}

	void SpaceshipListState::OnBackPressed()
	{
		m_nextState = m_previousState;
	}

	void SpaceshipListState::OnSpaceshipList(ServerConnection* server, const Packets::SpaceshipList& listPacket)
	{
		m_statusLabel->Show(false);
		m_titleLabel->Show(true);

		for (Ndk::BaseWidget* button : m_spaceshipButtons)
			DestroyWidget(button);

		m_spaceshipButtons.clear();

		for (const auto& spaceship : listPacket.spaceships)
		{
			Ndk::ButtonWidget* button = CreateWidget<Ndk::ButtonWidget>();
			button->UpdateText(Nz::SimpleTextDrawer::Draw(spaceship.name, 24));
			button->ResizeToContent();
			button->SetPadding(10.f, 10.f, 10.f, 10.f);
			button->OnButtonTrigger.Connect([&, name = spaceship.name](const Ndk::ButtonWidget* /*button*/)
			{
				m_nextState = std::make_shared<SpaceshipEditState>(GetStateData(), shared_from_this(), name);
			});

			m_spaceshipButtons.push_back(button);
		}

		LayoutWidgets();
	}

	void SpaceshipListState::UpdateStatus(const Nz::String& status, const Nz::Color& color)
	{
		m_statusLabel->Show(true);
		m_statusLabel->UpdateText(Nz::SimpleTextDrawer::Draw(status, 24, 0U, color));
		m_statusLabel->ResizeToContent();

		m_statusLabel->Center();
	}
}
