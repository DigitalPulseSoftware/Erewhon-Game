// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/DisconnectionState.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/StateMachine.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <Client/States/LoginState.hpp>
#include <cassert>

namespace ewn
{
	void DisconnectionState::Enter(Ndk::StateMachine& fsm)
	{
		AbstractState::Enter(fsm);

		StateData& stateData = GetStateData();

		m_accumulator = 0.f;
		m_dotCounter = 0;
		m_disconnected = false;
		m_statusSprite = Nz::TextSprite::New();
		m_timeout = 5.f;

		m_statusText = stateData.world2D->CreateEntity();
		m_statusText->AddComponent<Ndk::NodeComponent>();

		Ndk::GraphicsComponent& graphicsComponent = m_statusText->AddComponent<Ndk::GraphicsComponent>();
		graphicsComponent.Attach(m_statusSprite);

		if (stateData.server->IsConnected())
		{
			UpdateStatus("Disconnecting");

			ConnectSignal(stateData.server->OnDisconnected, this, &DisconnectionState::OnServerDisconnected);

			stateData.server->Disconnect();
		}
		else
			OnServerDisconnected(stateData.server, 0); //< Data is unused anyway
	}

	void DisconnectionState::Leave(Ndk::StateMachine& fsm)
	{
		AbstractState::Leave(fsm);

		m_statusSprite.Reset();
		m_statusText->Kill();
	}

	bool DisconnectionState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		m_accumulator += elapsedTime;
		m_timeout -= elapsedTime;
		if (m_disconnected || m_timeout < 0.f)
		{
			constexpr float messageTime = 0.2f;
			if (m_accumulator > messageTime)
			{
				if (m_shouldQuitApp)
					GetStateData().app->Quit();
				else
					fsm.ChangeState(std::make_shared<LoginState>(GetStateData()));
			}
		}
		else
		{
			if (m_accumulator > 0.3f)
			{
				m_accumulator -= 0.3f;

				if (++m_dotCounter >= 4)
					m_dotCounter = 0;

				UpdateStatus("Disconnecting" + Nz::String(m_dotCounter, '.'), Nz::Color::White, false);
			}
		}

		return true;
	}

	void DisconnectionState::LayoutWidgets()
	{
		Ndk::GraphicsComponent& graphicsComponent = m_statusText->GetComponent<Ndk::GraphicsComponent>();
		Ndk::NodeComponent& nodeComponent = m_statusText->GetComponent<Ndk::NodeComponent>();

		Nz::Boxf textBox = graphicsComponent.GetAABB();
		Nz::Vector2ui windowSize = GetStateData().window->GetSize();
		nodeComponent.SetPosition(windowSize.x / 2 - textBox.width / 2, windowSize.y / 2 - textBox.height / 2);
	}

	void DisconnectionState::OnServerDisconnected(ServerConnection* server, Nz::UInt32 /*data*/)
	{
		UpdateStatus("Disconnected", Nz::Color::White);

		m_disconnected =  true;
	}

	void DisconnectionState::UpdateStatus(const Nz::String& status, const Nz::Color& color, bool center)
	{
		assert(m_statusSprite);
		m_statusSprite->Update(Nz::SimpleTextDrawer::Draw(status, 24, 0U, color));

		if (center)
			LayoutWidgets();
	}
}
