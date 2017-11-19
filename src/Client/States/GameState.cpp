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
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/ParticleEmitterComponent.hpp>
#include <NDK/Components/ParticleGroupComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/VelocityComponent.hpp>
#include <NDK/Systems/RenderSystem.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
#include <random>

namespace ewn
{
	void GameState::Enter(Ndk::StateMachine& /*fsm*/)
	{
		if (Nz::Texture* background = Nz::TextureLibrary::Get("Background"); background && background->IsValid())
			m_stateData.world3D->GetSystem<Ndk::RenderSystem>().SetDefaultBackground(Nz::SkyboxBackground::New(background));
		else
			m_stateData.world3D->GetSystem<Ndk::RenderSystem>().SetDefaultBackground(Nz::ColorBackground::New(Nz::Color::Black));

		// Earth
		Nz::MeshRef earthMesh = Nz::Mesh::New();
		earthMesh->CreateStatic();
		earthMesh->BuildSubMesh(Nz::Primitive::UVSphere(1.f, 40, 40));

		Nz::MaterialRef earthMaterial = Nz::Material::New();
		earthMaterial->SetDiffuseMap("Assets/earth/earthmap1k.jpg");
		earthMaterial->SetShader("Basic");

		Nz::ModelRef earthModel = Nz::Model::New();
		earthModel->SetMesh(earthMesh);
		earthModel->SetMaterial(0, earthMaterial);

		m_earthEntity = m_stateData.world3D->CreateEntity();
		m_earthEntity->AddComponent<Ndk::GraphicsComponent>().Attach(earthModel);

		auto& earthNode = m_earthEntity->AddComponent<Ndk::NodeComponent>();
		earthNode.SetPosition(Nz::Vector3f::Forward() * 50.f);
		earthNode.SetRotation(Nz::EulerAnglesf(0.f, 180.f, 0.f));
		earthNode.SetScale(20.f);

		// Spaceship
		Nz::ModelParameters params;
		params.mesh.center = true;
		params.mesh.matrix.MakeTransform(Nz::Vector3f::Zero(), Nz::EulerAnglesf(0.f, 90.f, 0.f), Nz::Vector3f(0.01f));
		params.mesh.texCoordScale.Set(1.f, -1.f);
		params.material.shaderName = "Basic";

		Nz::ModelRef spaceshipModel = Nz::Model::New();
		spaceshipModel->LoadFromFile("Assets/spaceship/spaceship.obj", params);

		m_spaceshipTemplateEntity = m_stateData.world3D->CreateEntity();
		m_spaceshipTemplateEntity->AddComponent<Ndk::GraphicsComponent>().Attach(spaceshipModel);
		m_spaceshipTemplateEntity->AddComponent<Ndk::NodeComponent>();
		m_spaceshipTemplateEntity->GetComponent<Ndk::NodeComponent>().Move(Nz::Vector3f::Right() * 10.f);

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

		m_isCurrentlyRotating = false;
		m_rotationDirection.MakeZero();
		m_spaceshipRotation.MakeZero();
		m_spaceshipSpeed.MakeZero();

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
				m_cursorEntity->Enable(true);
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

				m_cursorEntity->Enable(false);
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

		m_cursorEntity->Enable(false);

		/*m_stateData.window->SetCursor(Nz::SystemCursor_None);
		Nz::Vector2ui windowSize = m_stateData.window->GetSize();
		Nz::Mouse::SetPosition(windowSize.x / 2, windowSize.y / 2, *m_stateData.window);*/

		/*eventHandler.OnMouseMoved.Connect([&](const Nz::EventHandler*, const Nz::WindowEvent::MouseMoveEvent& mouse)
		{
			m_spaceshipRotation.x -= mouse.deltaX * 0.1f;
			m_spaceshipRotation.y += mouse.deltaY * 0.1f;
		});*/

		m_onArenaStateSlot.Connect(m_stateData.app->OnArenaState, this, &GameState::OnArenaState);
		m_onControlSpaceshipSlot.Connect(m_stateData.app->OnControlSpaceship, this, &GameState::OnControlSpaceship);
		m_onCreateSpaceshipSlot.Connect(m_stateData.app->OnCreateSpaceship, this, &GameState::OnCreateSpaceship);
		m_onDeleteSpaceshipSlot.Connect(m_stateData.app->OnDeleteSpaceship, this, &GameState::OnDeleteSpaceship);

		m_inputClock.Restart();

		m_stateData.app->SendPacket(Packets::JoinArena());
	}

	void GameState::Leave(Ndk::StateMachine& /*fsm*/)
	{
		m_onArenaStateSlot.Disconnect();
		m_onControlSpaceshipSlot.Disconnect();
		m_onCreateSpaceshipSlot.Disconnect();
		m_onDeleteSpaceshipSlot.Disconnect();

		for (const auto& pair : m_serverIdToClient)
		{
			const Ndk::EntityHandle& spaceship = m_stateData.world3D->GetEntity(pair.second);
			spaceship->Kill();
		}

		m_cursorEntity->Kill();
		m_earthEntity->Kill();
		m_spaceshipTemplateEntity->Kill();
	}

	bool GameState::Update(Ndk::StateMachine& /*fsm*/, float elapsedTime)
	{
		auto& earthNode = m_earthEntity->GetComponent<Ndk::NodeComponent>();
		earthNode.Rotate(Nz::EulerAnglesf(0.f, 2.f * elapsedTime, 0.f));

		// Update and send input
		float inputElapsedTime = m_inputClock.GetSeconds();
		if (inputElapsedTime > 1.f / 60.f)
		{
			m_inputClock.Restart();

			UpdateInput(inputElapsedTime);
		}

		return true;
	}

	void GameState::OnArenaState(const Packets::ArenaState& arenaState)
	{
		for (const auto& spaceshipData : arenaState.spaceships)
		{
			assert(m_serverIdToClient.find(spaceshipData.id) != m_serverIdToClient.end());
			const Ndk::EntityHandle& spaceship = m_stateData.world3D->GetEntity(m_serverIdToClient[spaceshipData.id]);
			assert(spaceship);

			auto& spaceshipNode = spaceship->GetComponent<Ndk::NodeComponent>();
			spaceshipNode.SetPosition(spaceshipData.position);
			spaceshipNode.SetRotation(spaceshipData.rotation);
		}
	}

	void GameState::OnControlSpaceship(const Packets::ControlSpaceship& controlPacket)
	{
		assert(m_serverIdToClient.find(controlPacket.id) != m_serverIdToClient.end());
		const Ndk::EntityHandle& spaceship = m_stateData.world3D->GetEntity(m_serverIdToClient[controlPacket.id]);
		assert(spaceship);

		Ndk::NodeComponent& nodeComponent = m_stateData.camera3D->GetComponent<Ndk::NodeComponent>();
		nodeComponent.SetParent(spaceship);
		nodeComponent.SetPosition(Nz::Vector3f::Backward() * 8.f + Nz::Vector3f::Up() * 3.f);
		nodeComponent.SetRotation(Nz::EulerAnglesf(-10.f, 0.f, 0.f));

		m_controlledSpaceship = spaceship;
	}

	void GameState::OnCreateSpaceship(const Packets::CreateSpaceship& createPacket)
	{
		const Ndk::EntityHandle& spaceship = m_spaceshipTemplateEntity->Clone();

		auto& spaceshipNode = spaceship->GetComponent<Ndk::NodeComponent>();
		spaceshipNode.SetPosition(createPacket.position);
		spaceshipNode.SetRotation(createPacket.rotation);

		m_serverIdToClient[createPacket.id] = spaceship->GetId();
	}

	void GameState::OnDeleteSpaceship(const Packets::DeleteSpaceship& deletePacket)
	{
		assert(m_serverIdToClient.find(deletePacket.id) != m_serverIdToClient.end());
		const Ndk::EntityHandle& spaceship = m_stateData.world3D->GetEntity(m_serverIdToClient[deletePacket.id]);
		assert(spaceship);

		spaceship->Kill();

		m_serverIdToClient.erase(deletePacket.id);
	}

	void GameState::UpdateInput(float elapsedTime)
	{
		if (m_stateData.window->HasFocus())
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
				m_rotationDirection = Nz::Vector2f(m_rotationCursorPosition);
				if (m_rotationDirection.GetSquaredLength() < Nz::IntegralPow(50.f, 2))
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

		m_rotationDirection.x = Nz::Approach(m_rotationDirection.x, 0.f, 50.f);
		m_rotationDirection.y = Nz::Approach(m_rotationDirection.y, 0.f, 50.f);

		m_spaceshipRotation.x = Nz::Clamp(-m_rotationDirection.y, -200.f, 200.f);
		m_spaceshipRotation.y = Nz::Clamp(-m_rotationDirection.x, -200.f, 200.f);

		//m_spaceshipMovementNode.Move(elapsedTime * (m_spaceshipSpeed.x * Nz::Vector3f::Forward() + m_spaceshipSpeed.y * Nz::Vector3f::Left() + m_spaceshipSpeed.z * Nz::Vector3f::Up()));
		//m_spaceshipMovementNode.Rotate(Nz::EulerAnglesf(m_spaceshipRotation.x * elapsedTime, m_spaceshipRotation.y * elapsedTime, m_spaceshipRotation.z * elapsedTime));

		Packets::PlayerMovement movementPacket;
		movementPacket.direction = m_spaceshipSpeed;
		movementPacket.rotation = m_spaceshipRotation;

		m_stateData.app->SendPacket(movementPacket);
	}
}
