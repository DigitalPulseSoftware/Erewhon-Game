// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/ConnectedState.hpp>
#include <Client/ClientApplication.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/ConnectionLostState.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <cmath>

namespace ewn
{
	void ConnectedState::Enter(Ndk::StateMachine& fsm)
	{
		m_connectionLost = false;
		m_onConnectionInfoUpdateSlot.Connect(m_stateData.server->OnConnectionInfoUpdate, this, &ConnectedState::OnConnectionInfoUpdate);
		m_onRenderTargetSizeChangeSlot.Connect(m_stateData.window->OnRenderTargetSizeChange, [this](const Nz::RenderTarget*) { Layout(); });
	}

	void ConnectedState::Leave(Ndk::StateMachine& fsm)
	{
		m_onConnectionInfoUpdateSlot.Disconnect();
		m_onRenderTargetSizeChangeSlot.Disconnect();

		m_connectionLostEntity.Reset();
	}

	bool ConnectedState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!m_stateData.server->IsConnected())
		{
			fsm.ResetState(std::make_shared<BackgroundState>(m_stateData));
			fsm.PushState(std::make_shared<ConnectionLostState>(m_stateData));
			return false;
		}

		m_counter += elapsedTime;
		if (m_connectionLostEntity)
		{
			if (m_connectionLost)
			{
				float alpha = std::abs(std::sin(m_counter) * 255.f);
				m_connectionLostSprite->SetColor(Nz::Color(255, 0, 0, Nz::UInt8(alpha)));
			}
			else
			{
				float alpha = std::cos(m_counter) * 255.f;
				if (alpha > 0.f)
					m_connectionLostSprite->SetColor(Nz::Color(0, 255, 0, Nz::UInt8(alpha)));
				else
					m_connectionLostEntity.Reset();
			}
		}

		// Update connection informations every seconds
		if (m_infoRefreshClock.GetSeconds() >= 1.f)
		{
			m_stateData.server->RefreshInfos();

			m_infoRefreshClock.Restart();
		}

		return true;
	}

	void ConnectedState::Layout()
	{
		if (m_connectionLostEntity)
		{
			auto& entityNode = m_connectionLostEntity->GetComponent<Ndk::NodeComponent>();
			entityNode.SetPosition(m_stateData.window->GetSize().x - m_connectionLostSprite->GetSize().x - 10.f, 10.f);
		}
	}

	void ConnectedState::OnConnectionInfoUpdate(ServerConnection* server, const ServerConnection::ConnectionInfo& info)
	{
		std::cout << "Connected" << std::endl;
		std::cout << " - Ping: " << info.ping << std::endl;
		std::cout << " - Last received: " << ClientApplication::GetAppTime() - info.lastReceiveTime << std::endl;

		constexpr Nz::UInt64 connectionTimeout = 5'000;

		Nz::UInt64 timeSinceLastPacket = ClientApplication::GetAppTime() - info.lastReceiveTime;
		if (timeSinceLastPacket >= connectionTimeout)
		{
			if (!m_connectionLost)
				OnConnectionLost();
		}
		else
		{
			if (m_connectionLost)
				OnConnectionRetrieved();
		}
	}

	void ConnectedState::OnConnectionLost()
	{
		m_connectionLost = true;
		m_counter = 0.f;

		if (!m_connectionLostSprite)
		{
			const std::string& assetsFolder = m_stateData.app->GetConfig().GetStringOption("AssetsFolder");

			Nz::MaterialRef connectionLostMat = Nz::Material::New("Translucent2D");
			connectionLostMat->SetDiffuseMap(assetsFolder + "/sprites/connectionlost.png");

			m_connectionLostSprite = Nz::Sprite::New();
			m_connectionLostSprite->SetMaterial(std::move(connectionLostMat));
		}

		m_connectionLostSprite->SetColor(Nz::Color::Red);

		m_connectionLostEntity = m_stateData.world2D->CreateEntity();
		m_connectionLostEntity->AddComponent<Ndk::GraphicsComponent>().Attach(m_connectionLostSprite);
		m_connectionLostEntity->AddComponent<Ndk::NodeComponent>();

		Layout();
	}

	void ConnectedState::OnConnectionRetrieved()
	{
		m_connectionLost = false;
		m_counter = 0.f;

		m_connectionLostSprite->SetColor(Nz::Color::Green);
	}
}
