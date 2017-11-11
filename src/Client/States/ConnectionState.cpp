// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/ConnectionState.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <Ndk/StateMachine.hpp>
#include <Ndk/Components/GraphicsComponent.hpp>
#include <Ndk/Components/NodeComponent.hpp>
#include <Client/States/LoginState.hpp>
#include <cassert>

namespace ewn
{
	void ConnectionState::Enter(Ndk::StateMachine& /*fsm*/)
	{
		m_accumulator = 0.f;
		m_counter = 0;
		m_connected = Nz::Ternary_Unknown;
		m_statusSprite = Nz::TextSprite::New();

		m_statusText = m_stateData.world2D->CreateEntity();
		m_statusText->AddComponent<Ndk::NodeComponent>();

		Ndk::GraphicsComponent& graphicsComponent = m_statusText->AddComponent<Ndk::GraphicsComponent>();
		graphicsComponent.Attach(m_statusSprite);

		if (m_stateData.app->IsConnected())
		{
			OnServerConnected(0); //< Data is unused anyway
		}
		else
		{
			UpdateStatus("Connecting");

			m_onServerConnectedSlot.Connect(m_stateData.app->OnServerConnected, this, &ConnectionState::OnServerConnected);
			m_onServerDisconnectedSlot.Connect(m_stateData.app->OnServerDisconnected, this, &ConnectionState::OnServerDisconnected);
		}

		m_onTargetChangeSizeSlot.Connect(m_stateData.window->OnRenderTargetSizeChange, [this](const Nz::RenderTarget*) { CenterStatus(); });
	}

	void ConnectionState::Leave(Ndk::StateMachine& /*fsm*/)
	{
		m_onServerConnectedSlot.Disconnect();
		m_onServerDisconnectedSlot.Disconnect();
		m_onTargetChangeSizeSlot.Disconnect();
		m_statusSprite.Reset();
		m_statusText->Kill();
	}

	bool ConnectionState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		m_accumulator += elapsedTime;
		if (m_connected == Nz::Ternary_True)
		{
			constexpr float fadingOutTime = 2.f;

			float delta = (fadingOutTime - std::min(m_accumulator, fadingOutTime)) / fadingOutTime;

			m_statusSprite->SetColor(Nz::Color(255, 255, 255, Nz::UInt8(delta * 255.f)));
			if (m_accumulator > fadingOutTime)
				fsm.ChangeState(std::make_shared<LoginState>(m_stateData));
		}
		else if (m_connected == Nz::Ternary_Unknown)
		{
			if (m_accumulator > 0.5f)
			{
				m_accumulator -= 0.5f;

				if (++m_counter >= 4)
					m_counter = 0;

				UpdateStatus("Connecting" + Nz::String(m_counter, '.'), Nz::Color::White, false);
			}
		}

		return true;
	}

	void ConnectionState::CenterStatus()
	{
		Ndk::GraphicsComponent& graphicsComponent = m_statusText->GetComponent<Ndk::GraphicsComponent>();
		Ndk::NodeComponent& nodeComponent = m_statusText->GetComponent<Ndk::NodeComponent>();

		Nz::Boxf textBox = graphicsComponent.GetBoundingVolume().obb.localBox;
		Nz::Vector2ui windowSize = m_stateData.window->GetSize();
		nodeComponent.SetPosition(windowSize.x / 2 - textBox.width / 2, windowSize.y / 2 - textBox.height / 2);
	}

	void ConnectionState::OnServerConnected(Nz::UInt32 /*data*/)
	{
		UpdateStatus("Connected");

		m_accumulator = 0.f;
		m_connected = Nz::Ternary_True;
	}

	void ConnectionState::OnServerDisconnected(Nz::UInt32 /*data*/)
	{
		UpdateStatus("Connection failed", Nz::Color::Red);

		m_connected = Nz::Ternary_False;
	}

	void ConnectionState::UpdateStatus(const Nz::String& status, const Nz::Color& color, bool center)
	{
		assert(m_statusSprite);
		m_statusSprite->Update(Nz::SimpleTextDrawer::Draw(status, 24, 0U, color));

		if (center)
			CenterStatus();
	}
}
