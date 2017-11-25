// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
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
	void DisconnectionState::Enter(Ndk::StateMachine& /*fsm*/)
	{
		m_accumulator = 0.f;
		m_dotCounter = 0;
		m_disconnected = false;
		m_statusSprite = Nz::TextSprite::New();
		m_timeout = 5.f;

		m_statusText = m_stateData.world2D->CreateEntity();
		m_statusText->AddComponent<Ndk::NodeComponent>();

		Ndk::GraphicsComponent& graphicsComponent = m_statusText->AddComponent<Ndk::GraphicsComponent>();
		graphicsComponent.Attach(m_statusSprite);

		if (m_stateData.server->IsConnected())
		{
			UpdateStatus("Disconnecting");

			m_onServerDisconnectedSlot.Connect(m_stateData.server->OnDisconnected, this, &DisconnectionState::OnServerDisconnected);

			m_stateData.server->Disconnect();
		}
		else
			OnServerDisconnected(m_stateData.server, 0); //< Data is unused anyway

		m_onTargetChangeSizeSlot.Connect(m_stateData.window->OnRenderTargetSizeChange, [this](const Nz::RenderTarget*) { CenterStatus(); });
	}

	void DisconnectionState::Leave(Ndk::StateMachine& /*fsm*/)
	{
		m_onServerDisconnectedSlot.Disconnect();
		m_onTargetChangeSizeSlot.Disconnect();
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
				m_stateData.app->Quit();
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

	void DisconnectionState::CenterStatus()
	{
		Ndk::GraphicsComponent& graphicsComponent = m_statusText->GetComponent<Ndk::GraphicsComponent>();
		Ndk::NodeComponent& nodeComponent = m_statusText->GetComponent<Ndk::NodeComponent>();

		Nz::Boxf textBox = graphicsComponent.GetBoundingVolume().obb.localBox;
		Nz::Vector2ui windowSize = m_stateData.window->GetSize();
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
			CenterStatus();
	}
}
