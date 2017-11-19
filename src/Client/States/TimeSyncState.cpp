// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/TimeSyncState.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/StateMachine.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <Client/States/LoginState.hpp>
#include <cassert>

namespace ewn
{
	void TimeSyncState::Enter(Ndk::StateMachine& /*fsm*/)
	{
		m_accumulator = 0.f;
		m_connected = m_stateData.app->IsConnected();
		m_results.clear();
		m_statusSprite = Nz::TextSprite::New();

		m_statusText = m_stateData.world2D->CreateEntity();
		m_statusText->AddComponent<Ndk::NodeComponent>();

		Ndk::GraphicsComponent& graphicsComponent = m_statusText->AddComponent<Ndk::GraphicsComponent>();
		graphicsComponent.Attach(m_statusSprite);

		m_onServerDisconnectedSlot.Connect(m_stateData.app->OnServerDisconnected, this, &TimeSyncState::OnServerDisconnected);
		m_onTargetChangeSizeSlot.Connect(m_stateData.window->OnRenderTargetSizeChange, [this](const Nz::RenderTarget*) { CenterStatus(); });
		m_onTimeSyncResponseSlot.Connect(m_stateData.app->OnTimeSyncResponse, this, &TimeSyncState::OnTimeSyncResponse);

		m_expectedRequestId = 0;
		m_nextStepTime = 0.2f;
	}

	void TimeSyncState::Leave(Ndk::StateMachine& /*fsm*/)
	{
		m_onServerDisconnectedSlot.Disconnect();
		m_onTargetChangeSizeSlot.Disconnect();
		m_statusSprite.Reset();
		m_statusText->Kill();
	}

	bool TimeSyncState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!m_connected)
		{
			if (m_accumulator > 2.f)
				fsm.ChangeState(std::make_shared<ConnectionState>(m_stateData));

			return true;
		}

		m_accumulator += elapsedTime;
		if (m_accumulator >= m_nextStepTime)
		{
			Packets::TimeSyncRequest timeSyncRequest;
			timeSyncRequest.requestId = m_expectedRequestId;

			m_requestTime = m_stateData.app->GetAppTime();
			m_stateData.app->SendPacket(timeSyncRequest);

			m_nextStepTime += 2.f;
		}

		return true;
	}

	void TimeSyncState::CenterStatus()
	{
		Ndk::GraphicsComponent& graphicsComponent = m_statusText->GetComponent<Ndk::GraphicsComponent>();
		Ndk::NodeComponent& nodeComponent = m_statusText->GetComponent<Ndk::NodeComponent>();

		Nz::Boxf textBox = graphicsComponent.GetBoundingVolume().obb.localBox;
		Nz::Vector2ui windowSize = m_stateData.window->GetSize();
		nodeComponent.SetPosition(windowSize.x / 2 - textBox.width / 2, windowSize.y / 2 - textBox.height / 2);
	}

	void TimeSyncState::OnServerDisconnected(Nz::UInt32 /*data*/)
	{
		UpdateStatus("Connection lost", Nz::Color::Red);

		m_accumulator = 0.f;
		m_connected = false;
	}

	void TimeSyncState::OnTimeSyncResponse(const Packets::TimeSyncResponse& response)
	{
		if (response.requestId != m_expectedRequestId)
			return;

		m_results.push_back(m_stateData.app->GetAppTime() - m_requestTime);
		UpdateStatus("Syncing time with server (" + Nz::String::Number(m_results.back()) + ")");

		m_nextStepTime = m_accumulator + 1.f;
		m_expectedRequestId++;
	}

	void TimeSyncState::UpdateStatus(const Nz::String& status, const Nz::Color& color)
	{
		assert(m_statusSprite);
		m_statusSprite->Update(Nz::SimpleTextDrawer::Draw(status, 24, 0U, color));

		CenterStatus();
	}
}
