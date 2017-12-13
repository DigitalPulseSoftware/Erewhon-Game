// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

// THIS IS ONLY A TEST FOR SPACESHIP CONTROL, IT'S UGLY

#include <Client/States/GameState.hpp>
#include <Nazara/Core/Primitive.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <Nazara/Graphics/Model.hpp>
#include <Nazara/Graphics/ParticleDeclaration.hpp>
#include <Nazara/Graphics/ParticleFunctionController.hpp>
#include <Nazara/Graphics/ParticleFunctionRenderer.hpp>
#include <Nazara/Graphics/ParticleMapper.hpp>
#include <Nazara/Graphics/ParticleStruct.hpp>
#include <Nazara/Graphics/SkyboxBackground.hpp>
#include <Nazara/Utility/Mesh.hpp>
#include <NDK/Components/CollisionComponent3D.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/ParticleEmitterComponent.hpp>
#include <NDK/Components/ParticleGroupComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent3D.hpp>
#include <NDK/Systems/RenderSystem.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
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

		// Particle effect
#if 0
		Ndk::ParticleEmitterComponent& particleEmitter = m_spaceshipEntity->AddComponent<Ndk::ParticleEmitterComponent>();
		particleEmitter.SetEmissionRate(30.f);

		particleEmitter.SetSetupFunc([this](const Ndk::EntityHandle& emitterEntity, Nz::ParticleMapper& particleMapper, unsigned int count)
		{
			Nz::Vector3f pos = emitterEntity->GetComponent<Ndk::NodeComponent>().GetPosition();

			Nz::ParticleStruct_Billboard* billboards = static_cast<Nz::ParticleStruct_Billboard*>(particleMapper.GetPointer());
			for (unsigned int j = 0; j < count; ++j)
			{
				billboards[j].color = Nz::Color::White;
				billboards[j].life = 10.f;
				billboards[j].position = pos;
				billboards[j].rotation = 0.f;
				billboards[j].size = { 1.28f, 1.28f };
			}
		});

		Nz::MaterialRef fireMat = Nz::Material::New("Translucent3D");
		fireMat->EnableFaceCulling(true);
		fireMat->SetDiffuseMap("Assets/particles/fire_particle.png");
		// Additive blending for fire
		fireMat->SetDstBlend(Nz::BlendFunc_One);
		fireMat->SetSrcBlend(Nz::BlendFunc_SrcAlpha);

		Ndk::ParticleGroupComponent& particleGroup = m_spaceshipEntity->AddComponent<Ndk::ParticleGroupComponent>(5000, Nz::ParticleDeclaration::Get(Nz::ParticleLayout_Billboard));
		particleGroup.AddEmitter(m_spaceshipEntity);
		particleGroup.SetRenderer(Nz::ParticleFunctionRenderer::New([fireMat](const Nz::ParticleGroup& /*group*/, const Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId, Nz::AbstractRenderQueue* renderQueue)
		{
			auto colorPtr = mapper.GetComponentPtr<const Nz::Color>(Nz::ParticleComponent_Color);
			auto posPtr = mapper.GetComponentPtr<const Nz::Vector3f>(Nz::ParticleComponent_Position);
			auto rotPtr = mapper.GetComponentPtr<const float>(Nz::ParticleComponent_Rotation);
			auto sizePtr = mapper.GetComponentPtr<const Nz::Vector2f>(Nz::ParticleComponent_Size);

			renderQueue->AddBillboards(0, fireMat, endId - startId + 1, posPtr, sizePtr, rotPtr, colorPtr);
		}));
		particleGroup.AddController(Nz::ParticleFunctionController::New([](Nz::ParticleGroup& group, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId, float elapsedTime)
		{
			auto colorPtr = mapper.GetComponentPtr<Nz::Color>(Nz::ParticleComponent_Color);
			auto lifePtr = mapper.GetComponentPtr<float>(Nz::ParticleComponent_Life);

			for (unsigned int i = startId; i <= endId; ++i)
			{
				if ((lifePtr[i] -= elapsedTime) < 0.f)
					group.KillParticle(i);

				colorPtr[i].a = static_cast<Nz::UInt8>(Nz::Clamp(lifePtr[i] * 255.f, 0.f, 255.f));
			}
		}));
#endif

		//m_spaceshipEntity->GetComponent<Ndk::NodeComponent>().SetParent(m_spaceshipMovementNode);

		m_controlledEntity = std::numeric_limits<std::size_t>::max();
		m_inputAccumulator = 0.f;
		m_isCurrentlyRotating = false;
		m_lastInputTime = 0;
		m_lastStateId = 0;
		m_resetSnapshots = true;
		m_rotationDirection.MakeZero();
		m_spaceshipRotation.MakeZero();
		m_spaceshipSpeed.MakeZero();
		m_snapshotUpdateAccumulator = 0.f;

		Nz::Vector2ui windowSize = m_stateData.window->GetSize();
		Nz::Mouse::SetPosition(windowSize.x / 2, windowSize.y / 2, *m_stateData.window);
		Nz::EventHandler& eventHandler = m_stateData.window->GetEventHandler();
		eventHandler.OnMouseButtonPressed.Connect([&](const Nz::EventHandler*, const Nz::WindowEvent::MouseButtonEvent& event)
		{
			if (event.button == Nz::Mouse::Right)
			{
				m_isCurrentlyRotating = true;
				m_rotationCursorOrigin = Nz::Mouse::GetPosition(*m_stateData.window);
				m_rotationCursorPosition.MakeZero();

				m_stateData.window->SetCursor(Nz::SystemCursor_None);
				m_cursorEntity->Enable();
				m_cursorOrientationSprite->SetColor(Nz::Color(255, 255, 255, 0));
			}
		});

		eventHandler.OnMouseButtonReleased.Connect([&](const Nz::EventHandler*, const Nz::WindowEvent::MouseButtonEvent& event)
		{
			if (event.button == Nz::Mouse::Right)
			{
				m_isCurrentlyRotating = false;
				m_stateData.window->SetCursor(Nz::SystemCursor_Default);
				Nz::Mouse::SetPosition(m_rotationCursorOrigin.x, m_rotationCursorOrigin.y, *m_stateData.window);

				m_cursorEntity->Disable();
			}
		});

		eventHandler.OnMouseMoved.Connect([&](const Nz::EventHandler*, const Nz::WindowEvent::MouseMoveEvent& event)
		{
			if (m_isCurrentlyRotating)
			{
				constexpr int distMax = 200;

				m_rotationCursorPosition.x += event.deltaX;
				m_rotationCursorPosition.y += event.deltaY;
				if (m_rotationCursorPosition.GetSquaredLength() > Nz::IntegralPow(distMax, 2))
				{
					Nz::Vector2f tempCursor(m_rotationCursorPosition);
					tempCursor.Normalize();
					tempCursor *= float(distMax);
					m_rotationCursorPosition = Nz::Vector2i(tempCursor);
				}

				Nz::Vector2ui windowCenter = m_stateData.window->GetSize() / 2;

				// Position
				Ndk::NodeComponent& cursorNode = m_cursorEntity->GetComponent<Ndk::NodeComponent>();
				cursorNode.SetPosition(float(windowCenter.x + m_rotationCursorPosition.x), float(windowCenter.y + m_rotationCursorPosition.y));

				// Angle
				float cursorAngle = std::atan2(float(m_rotationCursorPosition.y), float(m_rotationCursorPosition.x));
				cursorNode.SetRotation(Nz::EulerAnglesf(0.f, 0.f, Nz::RadianToDegree(cursorAngle)));

				// Alpha
				float cursorAlpha = float(m_rotationCursorPosition.GetSquaredLength()) / Nz::IntegralPow(distMax, 2);
				m_cursorOrientationSprite->SetColor(Nz::Color(255, 255, 255, static_cast<Nz::UInt8>(std::min(cursorAlpha * 255.f, 255.f))));

				Nz::Mouse::SetPosition(windowCenter.x, windowCenter.y, *m_stateData.window);
			}
		});




		// Movement cursor
		Nz::MaterialRef cursorMat = Nz::Material::New("Translucent2D");
		cursorMat->SetDiffuseMap("Assets/cursor/orientation.png");

		m_cursorOrientationSprite = Nz::Sprite::New();
		m_cursorOrientationSprite->SetMaterial(cursorMat);
		m_cursorOrientationSprite->SetSize({ 32.f, 32.f });
		m_cursorOrientationSprite->SetOrigin(m_cursorOrientationSprite->GetSize() / 2.f);

		m_cursorEntity = m_stateData.world2D->CreateEntity();
		m_cursorEntity->AddComponent<Ndk::GraphicsComponent>().Attach(m_cursorOrientationSprite);
		m_cursorEntity->AddComponent<Ndk::NodeComponent>().SetPosition({ 200.f, 200.f, 0.f });

		m_cursorEntity->Disable();

		/*m_stateData.window->SetCursor(Nz::SystemCursor_None);
		Nz::Vector2ui windowSize = m_stateData.window->GetSize();
		Nz::Mouse::SetPosition(windowSize.x / 2, windowSize.y / 2, *m_stateData.window);*/

		/*eventHandler.OnMouseMoved.Connect([&](const Nz::EventHandler*, const Nz::WindowEvent::MouseMoveEvent& mouse)
		{
			m_spaceshipRotation.x -= mouse.deltaX * 0.1f;
			m_spaceshipRotation.y += mouse.deltaY * 0.1f;
		});
		*/

		m_cameraRotation = Nz::Vector3f::Zero();
		m_chatEnteringBox = nullptr;
		m_chatLines.resize(maxChatLines);
		m_interpolationFactor = 0;

		m_chatBox = m_stateData.canvas->Add<Ndk::TextAreaWidget>();
		m_chatBox->EnableBackground(false);
		//m_chatBox->SetBackgroundColor(Nz::Color(70, 8, 15, 20));
		m_chatBox->SetSize({ 320.f, maxChatLines * 30.f });
		m_chatBox->SetPosition({ 5.f, m_stateData.window->GetSize().y - 40.f - m_chatBox->GetSize().y - 5.f, 0.f });
		m_chatBox->SetTextColor(Nz::Color::White);
		m_chatBox->SetReadOnly(true);

		m_onChatMessageSlot.Connect(m_stateData.server->OnChatMessage, this, &GameState::OnChatMessage);
		m_onControlEntitySlot.Connect(m_stateData.server->OnControlEntity, this, &GameState::OnControlEntity);
		m_onKeyPressedSlot.Connect(m_stateData.window->GetEventHandler().OnKeyPressed, this, &GameState::OnKeyPressed);
		m_onTargetChangeSizeSlot.Connect(m_stateData.window->OnRenderTargetSizeChange, [this](const Nz::RenderTarget*) { m_chatBox->SetPosition({ 5.f, m_stateData.window->GetSize().y - 30 - m_chatBox->GetSize().y, 0.f }); });

		m_matchEntities.emplace(m_stateData.server, m_stateData.world3D);
		m_onEntityCreatedSlot.Connect(m_matchEntities->OnEntityCreated, this, &GameState::OnEntityCreated);
		m_onEntityDeletionSlot.Connect(m_matchEntities->OnEntityDelete, this, &GameState::OnEntityDelete);

		m_stateData.server->SendPacket(Packets::JoinArena());
	}

	void GameState::Leave(Ndk::StateMachine& /*fsm*/)
	{
		m_onChatMessageSlot.Disconnect();
		m_onControlEntitySlot.Disconnect();
		m_onKeyPressedSlot.Disconnect();
		m_onTargetChangeSizeSlot.Disconnect();

		m_matchEntities.reset();

		m_chatBox->Destroy();
		if (m_chatEnteringBox)
			m_chatEnteringBox->Destroy();

		m_cursorEntity->Kill();
	}

	bool GameState::Update(Ndk::StateMachine& /*fsm*/, float elapsedTime)
	{
		//auto& earthNode = m_earthEntity->GetComponent<Ndk::NodeComponent>();
		//earthNode.Rotate(Nz::EulerAnglesf(0.f, 2.f * elapsedTime, 0.f));

		// Update and send input
		m_inputAccumulator += elapsedTime;
		m_snapshotUpdateAccumulator += elapsedTime;

		constexpr float inputSendInterval = 1.f / 60.f;
		if (m_inputAccumulator > inputSendInterval)
		{
			m_inputAccumulator -= inputSendInterval;
			UpdateInput(inputSendInterval);
		}

		m_matchEntities->Update(elapsedTime);

		/*m_interpolationFactor = std::min(m_interpolationFactor + elapsedTime * 10.f, 3.0f);

		auto& cameraNode = m_stateData.camera3D->GetComponent<Ndk::NodeComponent>();
		Nz::Quaternionf camRot = cameraNode.GetRotation();

		for (std::size_t i = 0; i < m_serverEntities.size(); ++i)
		{
			const ServerEntity& entityData = m_serverEntities[i];
			if (!entityData.isValid)
				continue;

			if (i == m_controlledEntity && false)
				continue;

			Nz::Vector3f currentPosition = Nz::Lerp(entityData.oldPosition, entityData.newPosition, m_interpolationFactor);
			Nz::Quaternionf currentRotation = Nz::Quaternionf::Slerp(entityData.oldRotation, entityData.newRotation, m_interpolationFactor);

			auto& spaceshipNode = entityData.entity->GetComponent<Ndk::NodeComponent>();
			/*spaceshipNode.SetPosition(currentPosition);
			spaceshipNode.SetRotation(currentRotation);*/

			// Update text position
			/*auto& textGfx = entityData.textEntity->GetComponent<Ndk::GraphicsComponent>();
			auto& textNode = entityData.textEntity->GetComponent<Ndk::NodeComponent>();
			textNode.SetPosition(spaceshipNode.GetPosition() + camRot * Nz::Vector3f::Up() * 6.f + Nz::Vector3f::Right() * textGfx.GetBoundingVolume().obb.localBox.width / 2.f);
			textNode.SetRotation(cameraNode.GetRotation());
		}*/

		m_cameraRotation.x = Nz::Approach(m_cameraRotation.x, m_spaceshipRotation.x / 10.f, 10.f * elapsedTime);
		m_cameraRotation.y = Nz::Approach(m_cameraRotation.y, m_spaceshipRotation.y / 10.f, 10.f * elapsedTime);
		m_cameraRotation.z = Nz::Approach(m_cameraRotation.z, m_spaceshipRotation.z / 10.f, 10.f * elapsedTime);
		m_cameraNode.SetRotation(Nz::EulerAnglesf(m_cameraRotation.x, m_cameraRotation.y, m_cameraRotation.z));

		return true;
	}

	void GameState::ControlEntity(std::size_t entityId)
	{
		if (m_controlledEntity != entityId && m_controlledEntity != std::numeric_limits<std::size_t>::max())
		{
			auto& oldData = m_matchEntities->GetServerEntity(m_controlledEntity);
			oldData.textEntity->Enable();
		}

		if (m_matchEntities->IsServerEntityValid(entityId))
		{
			auto& data = m_matchEntities->GetServerEntity(entityId);

			// Don't show our own name
			data.textEntity->Disable();

			m_cameraNode.SetParent(data.entity->GetComponent<Ndk::NodeComponent>());

			Ndk::NodeComponent& cameraNode = m_stateData.camera3D->GetComponent<Ndk::NodeComponent>();
			cameraNode.SetParent(m_cameraNode);
			cameraNode.SetPosition(Nz::Vector3f::Backward() * 12.f + Nz::Vector3f::Up() * 5.f);
			cameraNode.SetRotation(Nz::EulerAnglesf(-10.f, 0.f, 0.f));
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

	void GameState::OnEntityDelete(ServerMatchEntities * entities, ServerMatchEntities::ServerEntity & entityData)
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
					chat.text = text;
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
		else if (event.code == Nz::Keyboard::Space)
		{
			m_stateData.server->SendPacket(Packets::PlayerShoot());
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

	void GameState::UpdateInput(float elapsedTime)
	{
		if (m_controlledEntity == std::numeric_limits<std::size_t>::max())
			return;

		if (m_stateData.window->HasFocus() && !m_chatEnteringBox)
		{
			constexpr float acceleration = 30.f;
			constexpr float strafeSpeed = 20.f;
			constexpr float jumpSpeed = 20.f;
			constexpr float rollSpeed = 150.f;

			float forwardSpeedModifier = 0.f;
			if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Z))
				forwardSpeedModifier += acceleration * elapsedTime;

			if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::S))
				forwardSpeedModifier -= acceleration * elapsedTime;

			float leftSpeedModifier = 0.f;
			if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Q))
				leftSpeedModifier += strafeSpeed * elapsedTime;

			if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::D))
				leftSpeedModifier -= strafeSpeed * elapsedTime;

			float jumpSpeedModifier = 0.f;
			if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::LShift))
				jumpSpeedModifier += jumpSpeed * elapsedTime;

			if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::LControl))
				jumpSpeedModifier -= jumpSpeed * elapsedTime;

			float rollSpeedModifier = 0.f;
			if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::A))
				rollSpeedModifier += rollSpeed * elapsedTime;

			if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::E))
				rollSpeedModifier -= rollSpeed * elapsedTime;

			// Rotation
			if (Nz::Mouse::IsButtonPressed(Nz::Mouse::Right))
			{
				m_rotationDirection = Nz::Vector2f(m_rotationCursorPosition) / 2.f;
				if (m_rotationDirection.GetSquaredLength() < Nz::IntegralPow(20.f, 2))
					m_rotationDirection.MakeZero();
			}

			m_spaceshipSpeed.x = Nz::Clamp(m_spaceshipSpeed.x + forwardSpeedModifier, -20.f, 20.f);
			m_spaceshipSpeed.y = Nz::Clamp(m_spaceshipSpeed.y + leftSpeedModifier, -15.f, 15.f);
			m_spaceshipSpeed.z = Nz::Clamp(m_spaceshipSpeed.z + jumpSpeedModifier, -15.f, 15.f);
			m_spaceshipRotation.z = Nz::Clamp(m_spaceshipRotation.z + rollSpeedModifier, -100.f, 100.f);
		}

		m_spaceshipSpeed.x = Nz::Approach(m_spaceshipSpeed.x, 0.f, 10.f * elapsedTime);
		m_spaceshipSpeed.y = Nz::Approach(m_spaceshipSpeed.y, 0.f, 10.f * elapsedTime);
		m_spaceshipSpeed.z = Nz::Approach(m_spaceshipSpeed.z, 0.f, 10.f * elapsedTime);
		m_spaceshipRotation.z = Nz::Approach(m_spaceshipRotation.z, 0.f, 50.f * elapsedTime);

		//m_controlledSpaceship->GetComponent<Ndk::NodeComponent>().SetRotation(Nz::EulerAnglesf(-m_spaceshipSpeed.x / 5.f, 0.f, m_spaceshipSpeed.y));

		m_spaceshipRotation.x = Nz::Clamp(-m_rotationDirection.y, -200.f, 200.f);
		m_spaceshipRotation.y = Nz::Clamp(-m_rotationDirection.x, -200.f, 200.f);

		m_rotationDirection.x = Nz::Approach(m_rotationDirection.x, 0.f, 200.f * elapsedTime);
		m_rotationDirection.y = Nz::Approach(m_rotationDirection.y, 0.f, 200.f * elapsedTime);

		//m_spaceshipMovementNode.Move(elapsedTime * (m_spaceshipSpeed.x * Nz::Vector3f::Forward() + m_spaceshipSpeed.y * Nz::Vector3f::Left() + m_spaceshipSpeed.z * Nz::Vector3f::Up()));
		//m_spaceshipMovementNode.Rotate(Nz::EulerAnglesf(m_spaceshipRotation.x * elapsedTime, m_spaceshipRotation.y * elapsedTime, m_spaceshipRotation.z * elapsedTime));

		Nz::UInt64 serverTime = m_stateData.server->EstimateServerTime();

		ClientInput input;
		input.inputTime = serverTime;
		input.movement = m_spaceshipSpeed;
		input.rotation = m_spaceshipRotation;

		m_predictedInputs.push_back(input);

		// Client-side prediction
		//ApplyInput(clientNode, m_lastInputTime, m_predictedInputs.back());

		m_lastInputTime = serverTime;

		// Send input to server
		Packets::PlayerMovement movementPacket;
		movementPacket.inputTime = serverTime;
		movementPacket.direction = m_spaceshipSpeed;
		movementPacket.rotation = m_spaceshipRotation;

		m_stateData.server->SendPacket(movementPacket);
	}

	void GameState::ApplyInput(Nz::Node& node, Nz::UInt64 lastInputTime, const ClientInput& input)
	{
		float inputElapsedTime = (lastInputTime != 0) ? (input.inputTime - lastInputTime) / 1000.f : 0.f;

		Nz::Vector3f totalMovement = inputElapsedTime * (input.movement.x * Nz::Vector3f::Forward() + input.movement.y * Nz::Vector3f::Left() + input.movement.z * Nz::Vector3f::Up());
		Nz::EulerAnglesf totalRotation = Nz::EulerAnglesf(input.rotation.x * inputElapsedTime, input.rotation.y * inputElapsedTime, input.rotation.z * inputElapsedTime);

		node.Move(totalMovement);
		node.Rotate(totalRotation);

		/*
		std::cout << "Prediction:" << std::endl;
		std::cout << "At " << serverTime << ": Move by " << totalMovement << " (final pos: " << clientNode.GetPosition() << ")\n";
		std::cout << "   " << serverTime << ": Rotate by " << totalRotation << " (final rot: " << clientNode.GetRotation().ToEulerAngles() << ')' << std::endl;
		*/
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
