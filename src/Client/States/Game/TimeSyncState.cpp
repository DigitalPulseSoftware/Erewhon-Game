// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/TimeSyncState.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/StateMachine.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <Client/States/ConnectedState.hpp>
#include <Client/States/Game/ArenaState.hpp>
#include <cassert>
#include <numeric>

namespace ewn
{
	void TimeSyncState::Enter(Ndk::StateMachine& fsm)
	{
		AbstractState::Enter(fsm);

		StateData& stateData = GetStateData();

		m_accumulator = 0.f;
		m_expectedRequestId = 0;
		m_finished = false;
		m_nextStepTime = 0.2f;
		m_pingAccumulator = 0;
		m_results.clear();
		m_statusSprite = Nz::TextSprite::New();

		m_statusText = stateData.world2D->CreateEntity();
		m_statusText->AddComponent<Ndk::NodeComponent>();

		Ndk::GraphicsComponent& graphicsComponent = m_statusText->AddComponent<Ndk::GraphicsComponent>();
		graphicsComponent.Attach(m_statusSprite);

		LayoutWidgets();

		ConnectSignal(stateData.server->OnTimeSyncResponse, this, &TimeSyncState::OnTimeSyncResponse);
	}

	void TimeSyncState::Leave(Ndk::StateMachine& fsm)
	{
		AbstractState::Leave(fsm);

		m_statusSprite.Reset();
		m_statusText->Kill();
	}

	bool TimeSyncState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		StateData& stateData = GetStateData();

		m_accumulator += elapsedTime;
		if (m_accumulator >= m_nextStepTime)
		{
			if (!m_finished)
			{
				Packets::TimeSyncRequest timeSyncRequest;
				timeSyncRequest.requestId = m_expectedRequestId;

				m_requestTime = stateData.app->GetAppTime();
				stateData.server->SendPacket(timeSyncRequest);

				m_nextStepTime += 1.f;
			}
			else
			{
				fsm.ResetState(std::make_shared<ConnectedState>(stateData));
				fsm.PushState(std::make_shared<ArenaState>(stateData, m_arenaIndex));
			}
		}

		return true;
	}

	void TimeSyncState::LayoutWidgets()
	{
		Ndk::GraphicsComponent& graphicsComponent = m_statusText->GetComponent<Ndk::GraphicsComponent>();
		Ndk::NodeComponent& nodeComponent = m_statusText->GetComponent<Ndk::NodeComponent>();

		Nz::Boxf textBox = graphicsComponent.GetAABB();
		Nz::Vector2ui windowSize = GetStateData().window->GetSize();
		nodeComponent.SetPosition(windowSize.x / 2 - textBox.width / 2, windowSize.y / 2 - textBox.height / 2);
	}

	void TimeSyncState::OnTimeSyncResponse(ServerConnection*, const Packets::TimeSyncResponse& response)
	{
		static constexpr std::size_t DesiredRequestCount = 30;

		if (response.requestId != m_expectedRequestId)
			return;

		StateData& stateData = GetStateData();

		Nz::UInt64 appTime = stateData.app->GetAppTime();
		Nz::UInt64 pingTime = appTime - m_requestTime;

		bool youngerThanServer = (response.serverTime >= appTime);
		if (response.requestId == 0)
		{
			// First request, determine if server is younger than user
			m_isClientYounger = youngerThanServer;
		}
		else if (youngerThanServer != m_isClientYounger)
		{
			// Oops, server crashed?
			m_expectedRequestId = 0;
			m_nextStepTime = m_accumulator + 1.f;
			m_pingAccumulator = 0;
			m_results.clear();

			UpdateStatus("Error in time synchronization, restarting process...", Nz::Color::Red);
			return;
		}

		Nz::UInt64 diff = (youngerThanServer) ? (response.serverTime - appTime) : (appTime - response.serverTime);
		diff += pingTime / 2;

		m_pingAccumulator += pingTime;

		m_results.push_back(diff);

		UpdateStatus("Syncing clock with server " + Nz::String::Number(m_results.size()) + "/" + Nz::String::Number(DesiredRequestCount));

		if (m_results.size() >= DesiredRequestCount)
		{
			Nz::UInt64 meanDiff = std::accumulate(m_results.begin(), m_results.end(), Nz::UInt64(0)) / m_results.size();
			Nz::UInt64 variance = std::accumulate(m_results.begin(), m_results.end(), Nz::UInt64(0), [meanDiff](Nz::UInt64 init, Nz::UInt64 delta)
			{
				return init + (delta - meanDiff) * (delta - meanDiff);
			});
			variance /= m_results.size() - 1;

			stateData.server->UpdateServerTimeDelta((m_isClientYounger) ? meanDiff : std::numeric_limits<Nz::UInt64>::max() - meanDiff);


			Nz::String standardDeviationStr;
			float standardDeviation = std::sqrt(float(variance));
			if (std::fmod(standardDeviation, 1.f) == 0.f)
				standardDeviationStr = Nz::String::Number(standardDeviation);
			else
				standardDeviationStr = "sqrt(" + Nz::String::Number(variance) + ')';

			UpdateStatus("Clock synchronized with server\n(mean ping: " + Nz::String::Number(m_pingAccumulator / m_results.size()) + " ± " + standardDeviationStr + "ms)");

			m_finished = true;
			m_nextStepTime = m_accumulator + 2.f;
		}
		else
		{
			m_nextStepTime = m_accumulator + 0.1f;
			m_expectedRequestId++;
		}
	}

	void TimeSyncState::UpdateStatus(const Nz::String& status, const Nz::Color& color)
	{
		assert(m_statusSprite);
		m_statusSprite->Update(Nz::SimpleTextDrawer::Draw(status, 24, 0U, color));

		LayoutWidgets();
	}
}
