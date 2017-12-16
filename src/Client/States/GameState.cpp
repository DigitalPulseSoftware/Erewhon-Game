// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

// THIS IS ONLY A TEST FOR SPACESHIP CONTROL, IT'S UGLY

#include <Client/States/GameState.hpp>
#include <Nazara/Audio/Sound.hpp>
#include <Nazara/Core/DynLib.hpp>
#include <Nazara/Core/Primitive.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <Nazara/Graphics/Model.hpp>
#include <Nazara/Graphics/ParticleDeclaration.hpp>
#include <Nazara/Graphics/ParticleFunctionController.hpp>
#include <Nazara/Graphics/ParticleFunctionRenderer.hpp>
#include <Nazara/Graphics/ParticleMapper.hpp>
#include <Nazara/Graphics/ParticleStruct.hpp>
#include <Nazara/Graphics/SkyboxBackground.hpp>
#include <Nazara/Renderer/DebugDrawer.hpp>
#include <Nazara/Utility/Mesh.hpp>
#include <Nazara/Utility/StaticMesh.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/CollisionComponent3D.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/ParticleEmitterComponent.hpp>
#include <NDK/Components/ParticleGroupComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Systems/RenderSystem.hpp>
#include <NDK/StateMachine.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/ConnectionLostState.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
#include <numeric>
#include <random>

namespace ewn
{
	static constexpr std::size_t maxChatLines = 15;

	void GameState::Enter(Ndk::StateMachine& /*fsm*/)
	{
		if (Nz::Texture* background = Nz::TextureLibrary::Get("Background"); background && background->IsValid())
			m_stateData.world3D->GetSystem<Ndk::RenderSystem>().SetDefaultBackground(Nz::SkyboxBackground::New(background));
		else
			m_stateData.world3D->GetSystem<Ndk::RenderSystem>().SetDefaultBackground(Nz::ColorBackground::New(Nz::Color::Black));

		m_chatEnteringBox = nullptr;
		m_chatLines.resize(maxChatLines);
		m_controlledEntity = std::numeric_limits<decltype(m_controlledEntity)>::max();
		m_isDisconnected = !m_stateData.server->IsConnected();

		m_chatBox = m_stateData.canvas->Add<Ndk::TextAreaWidget>();
		m_chatBox->EnableBackground(false);
		//m_chatBox->SetBackgroundColor(Nz::Color(70, 8, 15, 20));
		m_chatBox->SetSize({ 320.f, maxChatLines * 30.f });
		m_chatBox->SetTextColor(Nz::Color::White);
		m_chatBox->SetReadOnly(true);

		m_onChatMessageSlot.Connect(m_stateData.server->OnChatMessage, this, &GameState::OnChatMessage);
		m_onControlEntitySlot.Connect(m_stateData.server->OnControlEntity, this, &GameState::OnControlEntity);
		m_onKeyPressedSlot.Connect(m_stateData.window->GetEventHandler().OnKeyPressed, this, &GameState::OnKeyPressed);
		m_onTargetChangeSizeSlot.Connect(m_stateData.window->OnRenderTargetSizeChange, this, &GameState::OnRenderTargetSizeChange);

		m_matchEntities.emplace(m_stateData.app, m_stateData.server, m_stateData.world3D);
		m_onEntityCreatedSlot.Connect(m_matchEntities->OnEntityCreated, this, &GameState::OnEntityCreated);
		m_onEntityDeletionSlot.Connect(m_matchEntities->OnEntityDelete, this, &GameState::OnEntityDelete);

		m_stateData.server->SendPacket(Packets::JoinArena());

		OnRenderTargetSizeChange(m_stateData.window);
	}

	void GameState::Leave(Ndk::StateMachine& /*fsm*/)
	{
		m_onChatMessageSlot.Disconnect();
		m_onControlEntitySlot.Disconnect();
		m_onKeyPressedSlot.Disconnect();
		m_onIntegrityUpdateSlot.Disconnect();
		m_onTargetChangeSizeSlot.Disconnect();

		m_spaceshipController.reset();
		m_matchEntities.reset();

		m_chatBox->Destroy();
		if (m_chatEnteringBox)
			m_chatEnteringBox->Destroy();
	}

	bool GameState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!m_stateData.server->IsConnected())
		{
			fsm.ResetState(std::make_shared<BackgroundState>(m_stateData));
			fsm.PushState(std::make_shared<ConnectionLostState>(m_stateData));
			return false;
		}

		m_matchEntities->Update(elapsedTime);
		if (m_spaceshipController)
			m_spaceshipController->Update(elapsedTime);

		auto& cameraNode = m_stateData.camera3D->GetComponent<Ndk::NodeComponent>();
		Nz::Quaternionf camRot = cameraNode.GetRotation();

		for (std::size_t i = 0; i < m_matchEntities->GetServerEntityCount(); ++i)
		{
			if (!m_matchEntities->IsServerEntityValid(i))
				continue;

			const ServerMatchEntities::ServerEntity& entityData = m_matchEntities->GetServerEntity(i);

			auto& spaceshipNode = entityData.entity->GetComponent<Ndk::NodeComponent>();

			// Update text position
			if (entityData.textEntity)
			{
				auto& textGfx = entityData.textEntity->GetComponent<Ndk::GraphicsComponent>();
				auto& textNode = entityData.textEntity->GetComponent<Ndk::NodeComponent>();
				textNode.SetPosition(spaceshipNode.GetPosition() + camRot * Nz::Vector3f::Up() * 6.f + Nz::Vector3f::Right() * textGfx.GetBoundingVolume().obb.localBox.width / 2.f);
				textNode.SetRotation(cameraNode.GetRotation());
			}
		}

		return true;
	}

	void GameState::ControlEntity(std::size_t entityId)
	{
		if (m_controlledEntity != entityId && m_controlledEntity != std::numeric_limits<std::size_t>::max())
		{
			auto& oldData = m_matchEntities->GetServerEntity(m_controlledEntity);
			if (oldData.textEntity)
				oldData.textEntity->Enable();

			m_spaceshipController.reset();
		}

		if (m_matchEntities->IsServerEntityValid(entityId))
		{
			auto& data = m_matchEntities->GetServerEntity(entityId);

			// Don't show our own name
			if (data.textEntity)
				data.textEntity->Disable();

			m_spaceshipController.emplace(m_stateData.app, m_stateData.server, *m_stateData.window, *m_stateData.world2D, m_stateData.camera3D, data.entity);
		}

		m_controlledEntity = entityId;
	}

	void GameState::OnChatMessage(ServerConnection*, const Packets::ChatMessage & chatMessage)
	{
		PrintMessage(chatMessage.message);
	}

	void GameState::OnControlEntity(ServerConnection*, const Packets::ControlEntity& controlPacket)
	{
		ControlEntity(controlPacket.id);
	}

	void GameState::OnEntityCreated(ServerMatchEntities* /*entities*/, ServerMatchEntities::ServerEntity& entityData)
	{
		if (entityData.serverId == m_controlledEntity)
			ControlEntity(m_controlledEntity);
	}

	void GameState::OnEntityDelete(ServerMatchEntities* entities, ServerMatchEntities::ServerEntity & entityData)
	{
		if (entityData.serverId == m_controlledEntity)
			ControlEntity(std::numeric_limits<std::size_t>::max());
	}

	void GameState::OnKeyPressed(const Nz::EventHandler* /*eventHandler*/, const Nz::WindowEvent::KeyEvent& event)
	{
		if (event.code == Nz::Keyboard::Return)
		{
			if (m_chatEnteringBox)
			{
				Nz::String text = m_chatEnteringBox->GetText();

				if (!text.IsEmpty())
				{
					Packets::PlayerChat chat;
					chat.text = text.ToStdString();
					m_stateData.server->SendPacket(chat);
				}

				m_chatEnteringBox->Destroy();
				m_chatEnteringBox = nullptr;
				return;
			}

			m_chatEnteringBox = m_stateData.canvas->Add<Ndk::TextAreaWidget>();
			m_chatEnteringBox->EnableBackground(true);
			m_chatEnteringBox->SetBackgroundColor(Nz::Color(0, 0, 0, 150));
			m_chatEnteringBox->SetSize({ float(m_stateData.window->GetSize().x), 40.f });
			m_chatEnteringBox->SetPosition({ 0.f, m_stateData.window->GetSize().y - m_chatEnteringBox->GetSize().y - 5.f, 0.f });
			m_chatEnteringBox->SetTextColor(Nz::Color::White);
			m_chatEnteringBox->SetFocus();
		}
		else if (event.code == Nz::Keyboard::F1)
		{
			m_matchEntities->EnableSnapshotHandling(!m_matchEntities->IsSnapshotHandlingEnabled());
			if (m_matchEntities->IsSnapshotHandlingEnabled())
				PrintMessage("INFO: Sync enabled");
			else
				PrintMessage("INFO: Sync disabled");
		}
	}

	void GameState::OnRenderTargetSizeChange(const Nz::RenderTarget* renderTarget)
	{
		m_chatBox->SetPosition({ 5.f, renderTarget->GetSize().y - 30 - m_chatBox->GetSize().y, 0.f });
	}

	void GameState::PrintMessage(const std::string& message)
	{
		std::cout << message << std::endl;

		m_chatLines.emplace_back(message);
		if (m_chatLines.size() > maxChatLines)
			m_chatLines.erase(m_chatLines.begin());

		m_chatBox->Clear();
		for (const Nz::String& message : m_chatLines)
			m_chatBox->AppendText(message + "\n");
	}
}
