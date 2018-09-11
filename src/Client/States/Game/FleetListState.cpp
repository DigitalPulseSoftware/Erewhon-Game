// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/FleetListState.hpp>
#include <Nazara/Core/String.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/StateMachine.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Client/States/Game/FleetEditState.hpp>
#include <cassert>

namespace ewn
{
	void FleetListState::Enter(Ndk::StateMachine& fsm)
	{
		AbstractState::Enter(fsm);

		StateData& stateData = GetStateData();

		m_statusLabel = CreateWidget<Ndk::LabelWidget>();

		m_titleLabel = CreateWidget<Ndk::LabelWidget>();
		m_titleLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Fleets:", 24));
		//m_titleLabel->ResizeToContent();

		m_backButton = CreateWidget<Ndk::ButtonWidget>();
		m_backButton->UpdateText(Nz::SimpleTextDrawer::Draw("Back to main menu", 24));
		//m_backButton->SetPadding(10.f, 10.f, 10.f, 10.f);
		//m_backButton->ResizeToContent();
		m_backButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnBackPressed();
		});

		m_createButton = CreateWidget<Ndk::ButtonWidget>();
		m_createButton->UpdateText(Nz::SimpleTextDrawer::Draw("+", 24));
		//m_createButton->ResizeToContent();
		//m_createButton->SetPadding(20.f, 10.f, 20.f, 10.f);
		m_createButton->OnButtonTrigger.Connect([&](const Ndk::ButtonWidget* /*button*/)
		{
			StateData& stateData = GetStateData();
			stateData.fsm->ChangeState(std::make_shared<FleetEditState>(stateData, shared_from_this(), std::string()));
		});

		LayoutWidgets();

		ConnectSignal(stateData.server->OnFleetList, this, &FleetListState::OnFleetList);

		QueryFleets();
	}

	void FleetListState::Leave(Ndk::StateMachine& fsm)
	{
		AbstractState::Leave(fsm);

		m_fleetButtons.clear();
	}

	void FleetListState::LayoutWidgets()
	{
		Nz::Vector2f canvasSize = GetStateData().canvas->GetSize();

		m_statusLabel->Center();

		m_backButton->CenterHorizontal();
		m_backButton->SetPosition({ m_backButton->GetPosition().x, canvasSize.y - m_backButton->GetSize().y * 2.f, 0.f });

		constexpr float margin = 20.f;

		float totalHeight = 0.f;
		for (Ndk::BaseWidget* button : m_fleetButtons)
			totalHeight += button->GetSize().y + margin;

		totalHeight += m_createButton->GetSize().y + margin;

		float buttonTop = canvasSize.y / 2.f - totalHeight / 2.f;

		m_titleLabel->CenterHorizontal();
		m_titleLabel->SetPosition(m_titleLabel->GetPosition().x, buttonTop - m_titleLabel->GetSize().y - 5.f);

		float buttonSpace = totalHeight / (m_fleetButtons.size() + 1);
		for (std::size_t i = 0; i < m_fleetButtons.size(); ++i)
		{
			Ndk::BaseWidget* button = m_fleetButtons[i];
			button->CenterHorizontal();
			button->SetPosition(button->GetPosition().x, buttonTop + i * buttonSpace);
		}

		m_createButton->CenterHorizontal();
		m_createButton->SetPosition(m_createButton->GetPosition().x, buttonTop + m_fleetButtons.size() * buttonSpace);
	}

	void FleetListState::QueryFleets()
{
		m_titleLabel->Show(false);

		UpdateStatus("Loading fleets...");
		GetStateData().server->SendPacket(Packets::QueryFleetList());
	}

	void FleetListState::OnBackPressed()
	{
		GetStateData().fsm->ChangeState(m_previousState);
	}

	void FleetListState::OnFleetList(ServerConnection* server, const Packets::FleetList& listPacket)
	{
		m_statusLabel->Show(false);
		m_titleLabel->Show(true);

		for (Ndk::BaseWidget* button : m_fleetButtons)
			DestroyWidget(button);

		m_fleetButtons.clear();

		for (const auto& spaceship : listPacket.fleets)
		{
			Ndk::ButtonWidget* button = CreateWidget<Ndk::ButtonWidget>();
			button->UpdateText(Nz::SimpleTextDrawer::Draw(spaceship.name, 24));
			//button->ResizeToContent();
			//button->SetPadding(10.f, 10.f, 10.f, 10.f);
			button->OnButtonTrigger.Connect([&, name = spaceship.name](const Ndk::ButtonWidget* /*button*/)
			{
				StateData& stateData = GetStateData();
				stateData.fsm->ChangeState(std::make_shared<FleetEditState>(GetStateData(), shared_from_this(), name));
			});

			m_fleetButtons.push_back(button);
		}

		LayoutWidgets();
	}

	void FleetListState::UpdateStatus(const Nz::String& status, const Nz::Color& color)
	{
		m_statusLabel->Show(true);
		m_statusLabel->UpdateText(Nz::SimpleTextDrawer::Draw(status, 24, 0U, color));
		//m_statusLabel->ResizeToContent();

		m_statusLabel->Center();
	}
}
