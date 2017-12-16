// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/SpaceshipController.hpp>
#include <Nazara/Core/CallOnExit.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/LuaAPI.hpp>
#include <Client/ClientApplication.hpp>

namespace ewn
{
	SpaceshipController::SpaceshipController(ClientApplication* app, ServerConnection* server, Nz::RenderWindow& window, Ndk::World& world2D, const Ndk::EntityHandle& camera, const Ndk::EntityHandle& spaceship) :
	m_app(app),
	m_server(server),
	m_window(window),
	m_camera(camera),
	m_spaceship(spaceship),
	m_lastShootTime(0),
	m_executeScript(false),
	m_isCurrentlyRotating(false),
	m_inputAccumulator(0.f)
	{
		m_shootSound.SetBuffer(Nz::SoundBufferLibrary::Get("ShootSound"));
		m_shootSound.EnableSpatialization(false);

		Nz::Vector2ui windowSize = m_window.GetSize();
		Nz::Mouse::SetPosition(windowSize.x / 2, windowSize.y / 2, m_window);

		Nz::EventHandler& eventHandler = m_window.GetEventHandler();

		// Connect every slot
		m_onKeyPressedSlot.Connect(eventHandler.OnKeyPressed, this, &SpaceshipController::OnKeyPressed);
		m_onKeyReleasedSlot.Connect(eventHandler.OnKeyReleased, this, &SpaceshipController::OnKeyReleased);
		m_onIntegrityUpdateSlot.Connect(m_server->OnIntegrityUpdate, this, &SpaceshipController::OnIntegrityUpdate);
		m_onLostFocusSlot.Connect(eventHandler.OnLostFocus, this, &SpaceshipController::OnLostFocus);
		m_onMouseButtonPressedSlot.Connect(eventHandler.OnMouseButtonPressed, this, &SpaceshipController::OnMouseButtonPressed);
		m_onMouseButtonReleasedSlot.Connect(eventHandler.OnMouseButtonReleased, this, &SpaceshipController::OnMouseButtonReleased);
		m_onMouseMovedSlot.Connect(eventHandler.OnMouseMoved, this, &SpaceshipController::OnMouseMoved);
		m_onTargetChangeSizeSlot.Connect(m_window.OnRenderTargetSizeChange, this, &SpaceshipController::OnRenderTargetSizeChange);

		LoadSprites(world2D);
		OnRenderTargetSizeChange(&m_window);

		m_cameraNode.SetParent(m_spaceship->GetComponent<Ndk::NodeComponent>());

		Ndk::NodeComponent& cameraNode = m_camera->GetComponent<Ndk::NodeComponent>();
		cameraNode.SetParent(m_cameraNode);
		cameraNode.SetPosition(Nz::Vector3f::Backward() * 12.f + Nz::Vector3f::Up() * 5.f);
		cameraNode.SetRotation(Nz::EulerAnglesf(-10.f, 0.f, 0.f));

		// Load client script
		LoadScript();
	}

	SpaceshipController::~SpaceshipController()
	{
		m_cameraNode.SetParent(nullptr);
	}

	void SpaceshipController::Update(float elapsedTime)
	{
		// Update and send input
		m_inputAccumulator += elapsedTime;

		constexpr float inputSendInterval = 1.f / 60.f;
		if (m_inputAccumulator > inputSendInterval)
		{
			m_inputAccumulator -= inputSendInterval;
			UpdateInput(inputSendInterval);
		}

		// Compute crosshair position (according to projectile path projection)
		auto& cameraComponent = m_camera->GetComponent<Ndk::CameraComponent>();
		auto& entityNode = m_spaceship->GetComponent<Ndk::NodeComponent>();

		Nz::Vector4f worldPosition(entityNode.GetPosition() + entityNode.GetForward() * 150.f, 1.f);
		worldPosition = cameraComponent.GetViewMatrix() * worldPosition;
		worldPosition = cameraComponent.GetProjectionMatrix() * worldPosition;
		worldPosition /= worldPosition.w;

		Nz::Vector3f screenPosition(worldPosition.x * 0.5f + 0.5f, -worldPosition.y * 0.5f + 0.5f, worldPosition.z * 0.5f + 0.5f);
		screenPosition.x *= m_window.GetSize().x;
		screenPosition.y *= m_window.GetSize().y;

		m_crosshairEntity->GetComponent<Ndk::NodeComponent>().SetPosition(screenPosition);
	}

	void SpaceshipController::OnKeyPressed(const Nz::EventHandler* /*eventHandler*/, const Nz::WindowEvent::KeyEvent& event)
	{
		if (event.code == Nz::Keyboard::F5)
			LoadScript();
		else
		{
			if (m_executeScript)
			{
				if (m_controlScript.GetGlobal("OnKeyPressed") == Nz::LuaType_Function)
				{
					PushToLua(event);

					if (!m_controlScript.Call(1))
						std::cerr << "OnKeyPressed failed: " << m_controlScript.GetLastError() << std::endl;
				}
			}
		}
	}

	void SpaceshipController::OnKeyReleased(const Nz::EventHandler* /*eventHandler*/, const Nz::WindowEvent::KeyEvent& event)
	{
		if (m_executeScript)
		{
			if (m_controlScript.GetGlobal("OnKeyReleased") == Nz::LuaType_Function)
			{
				PushToLua(event);

				if (!m_controlScript.Call(1))
					std::cerr << "OnKeyReleased failed: " << m_controlScript.GetLastError() << std::endl;
			}
		}
	}

	void SpaceshipController::OnLostFocus(const Nz::EventHandler* /*eventHandler*/)
	{
		if (m_executeScript)
		{
			if (m_controlScript.GetGlobal("OnLostFocus") == Nz::LuaType_Function)
			{
				if (!m_controlScript.Call(0))
					std::cerr << "OnLostFocus failed: " << m_controlScript.GetLastError() << std::endl;
			}
		}
	}

	void SpaceshipController::OnIntegrityUpdate(ServerConnection* /*server*/, const Packets::IntegrityUpdate& integrityUpdate)
	{
		float integrityPct = integrityUpdate.integrityValue / 255.f;

		m_healthBarSprite->SetSize({ integrityPct * 256.f, 32.f });
	}

	void SpaceshipController::OnMouseButtonPressed(const Nz::EventHandler* /*eventHandler*/, const Nz::WindowEvent::MouseButtonEvent& event)
	{
		if (m_executeScript)
		{
			if (m_controlScript.GetGlobal("OnMouseButtonPressed") == Nz::LuaType_Function)
			{
				PushToLua(event);

				if (!m_controlScript.Call(1))
					std::cerr << "OnMouseButtonPressed failed: " << m_controlScript.GetLastError() << std::endl;
			}
		}
	}

	void SpaceshipController::OnMouseButtonReleased(const Nz::EventHandler* /*eventHandler*/, const Nz::WindowEvent::MouseButtonEvent& event)
	{
		if (m_executeScript)
		{
			if (m_controlScript.GetGlobal("OnMouseButtonReleased") == Nz::LuaType_Function)
			{
				PushToLua(event);

				if (!m_controlScript.Call(1))
					std::cerr << "OnMouseButtonReleased failed: " << m_controlScript.GetLastError() << std::endl;
			}
		}
	}

	void SpaceshipController::OnMouseMoved(const Nz::EventHandler* /*eventHandler*/, const Nz::WindowEvent::MouseMoveEvent& event)
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

			Nz::Vector2ui windowCenter = m_window.GetSize() / 2;

			// Position
			Ndk::NodeComponent& cursorNode = m_cursorEntity->GetComponent<Ndk::NodeComponent>();
			cursorNode.SetPosition(float(windowCenter.x + m_rotationCursorPosition.x), float(windowCenter.y + m_rotationCursorPosition.y));

			// Angle
			float cursorAngle = std::atan2(float(m_rotationCursorPosition.y), float(m_rotationCursorPosition.x));
			cursorNode.SetRotation(Nz::EulerAnglesf(0.f, 0.f, Nz::RadianToDegree(cursorAngle)));

			// Alpha
			float cursorAlpha = float(m_rotationCursorPosition.GetSquaredLength()) / Nz::IntegralPow(distMax, 2);
			m_cursorOrientationSprite->SetColor(Nz::Color(255, 255, 255, static_cast<Nz::UInt8>(std::min(cursorAlpha * 255.f, 255.f))));

			Nz::Mouse::SetPosition(windowCenter.x, windowCenter.y, m_window);
		}
	}

	void SpaceshipController::OnRenderTargetSizeChange(const Nz::RenderTarget* renderTarget)
	{
		m_healthBarEntity->GetComponent<Ndk::NodeComponent>().SetPosition({ renderTarget->GetSize().x - 300.f, renderTarget->GetSize().y - 70.f, 0.f });
	}

	void SpaceshipController::LoadScript()
	{
		m_controlScript = Nz::LuaInstance();
		m_executeScript = true;

		std::cout << "Loading spaceshipcontroller.lua" << std::endl;
		if (!m_controlScript.ExecuteFromFile("spaceshipcontroller.lua"))
		{
			std::cerr << "Failed to load spaceshipcontroller.lua: " << m_controlScript.GetLastError() << std::endl;
			m_executeScript = false;
			return;
		}

		// Check existence of some functions
		if (m_controlScript.GetGlobal("UpdateInput") != Nz::LuaType_Function)
		{
			std::cerr << "spaceshipcontroller.lua: UpdateInput is not a valid function!" << std::endl;
			m_executeScript = false;
		}
		m_controlScript.Pop();

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			if (state.CheckBoolean(1))
			{
				m_isCurrentlyRotating = true;
				m_rotationCursorOrigin = Nz::Mouse::GetPosition(m_window);
				m_rotationCursorPosition.MakeZero();

				m_window.SetCursor(Nz::SystemCursor_None);
				m_cursorEntity->Enable();
				m_cursorOrientationSprite->SetColor(Nz::Color(255, 255, 255, 0));
			}
			else
			{
				m_rotationCursorPosition.MakeZero();
				m_isCurrentlyRotating = false;
				m_window.SetCursor(Nz::SystemCursor_Default);
				Nz::Mouse::SetPosition(m_rotationCursorOrigin.x, m_rotationCursorOrigin.y, m_window);

				m_cursorEntity->Disable();
			}
			return 0;
		});
		m_controlScript.SetGlobal("EnableRotation");

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			state.PushTable(0, 2);
				state.PushField("x", m_rotationCursorPosition.x);
				state.PushField("y", m_rotationCursorPosition.y);

			return 1;
		});
		m_controlScript.SetGlobal("GetRotation");

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			Shoot();
			return 0;
		});
		m_controlScript.SetGlobal("Shoot");
	}

	void SpaceshipController::LoadSprites(Ndk::World& world2D)
	{
		// Movement cursor
		{
			Nz::MaterialRef cursorMat = Nz::Material::New("Translucent2D");
			cursorMat->SetDiffuseMap("Assets/cursor/orientation.png");

			m_cursorOrientationSprite = Nz::Sprite::New();
			m_cursorOrientationSprite->SetMaterial(cursorMat);
			m_cursorOrientationSprite->SetOrigin(m_cursorOrientationSprite->GetSize() / 2.f);
			m_cursorOrientationSprite->SetSize({ 32.f, 32.f });

			m_cursorEntity = world2D.CreateEntity();
			m_cursorEntity->AddComponent<Ndk::GraphicsComponent>().Attach(m_cursorOrientationSprite);
			m_cursorEntity->AddComponent<Ndk::NodeComponent>().SetPosition({ 200.f, 200.f, 0.f });

			m_cursorEntity->Disable();
		}

		// Crosshair
		{
			Nz::MaterialRef cursorMat = Nz::Material::New("Translucent2D");
			cursorMat->SetDiffuseMap("Assets/weapons/crosshair.png");

			Nz::SpriteRef crosshairSprite = Nz::Sprite::New();
			crosshairSprite->SetMaterial(cursorMat);
			crosshairSprite->SetSize({ 32.f, 32.f });
			crosshairSprite->SetOrigin(m_cursorOrientationSprite->GetSize() / 2.f);

			m_crosshairEntity = world2D.CreateEntity();
			m_crosshairEntity->AddComponent<Ndk::GraphicsComponent>().Attach(crosshairSprite);
			m_crosshairEntity->AddComponent<Ndk::NodeComponent>();
		}

		// Health bar
		{
			Nz::MaterialRef healthBarMat = Nz::Material::New();
			healthBarMat->EnableDepthBuffer(false);
			healthBarMat->EnableFaceCulling(false);

			Nz::SpriteRef healthBarBackground = Nz::Sprite::New();
			healthBarBackground->SetColor(Nz::Color::Black);
			healthBarBackground->SetOrigin({ 2.f, 2.f, 0.f });
			healthBarBackground->SetMaterial(healthBarMat);
			healthBarBackground->SetSize({ 256.f + 4.f, 32.f + 4.f });

			Nz::SpriteRef healthBarEmptySprite = Nz::Sprite::New();
			healthBarEmptySprite->SetSize({ 256.f, 32.f });
			healthBarEmptySprite->SetMaterial(healthBarMat);

			m_healthBarSprite = Nz::Sprite::New();
			m_healthBarSprite->SetCornerColor(Nz::RectCorner_LeftTop, Nz::Color::Orange);
			m_healthBarSprite->SetCornerColor(Nz::RectCorner_RightTop, Nz::Color::Orange);
			m_healthBarSprite->SetCornerColor(Nz::RectCorner_LeftBottom, Nz::Color::Yellow);
			m_healthBarSprite->SetCornerColor(Nz::RectCorner_RightBottom, Nz::Color::Yellow);
			m_healthBarSprite->SetMaterial(healthBarMat);
			m_healthBarSprite->SetSize({ 256.f, 32.f });

			m_healthBarEntity = world2D.CreateEntity();
			auto& crosshairGhx = m_healthBarEntity->AddComponent<Ndk::GraphicsComponent>();
			m_healthBarEntity->AddComponent<Ndk::NodeComponent>();

			crosshairGhx.Attach(healthBarBackground, 0);
			crosshairGhx.Attach(healthBarEmptySprite, 1);
			crosshairGhx.Attach(m_healthBarSprite, 2);
		}
	}

	void SpaceshipController::Shoot()
	{
		Nz::UInt64 currentTime = m_app->GetAppTime();
		if (currentTime - m_lastShootTime < 500)
			return;

		m_lastShootTime = currentTime;
		m_shootSound.Play();

		m_server->SendPacket(Packets::PlayerShoot());
	}

	void SpaceshipController::UpdateInput(float elapsedTime)
	{
		if (m_executeScript)
		{
			m_controlScript.GetGlobal("UpdateInput");
			m_controlScript.Push(elapsedTime);
			if (m_controlScript.Call(1, 2))
			{
				auto RetrieveVector3 = [&](int index) -> Nz::Vector3f
				{
					m_controlScript.CheckType(index, Nz::LuaType_Table);

					float x = m_controlScript.CheckField<float>("x", index);
					float y = m_controlScript.CheckField<float>("y", index);
					float z = m_controlScript.CheckField<float>("z", index);

					return { x, y, z };
				};


				// Use some RRID
				Nz::CallOnExit resetLuaStack([&]()
				{
					m_controlScript.Pop(m_controlScript.GetStackTop());
				});
				Nz::Vector3f movement;
				Nz::Vector3f rotation;
				try
				{
					movement = RetrieveVector3(1);
					rotation = RetrieveVector3(2);
				}
				catch (const std::exception&)
				{
					std::cerr << "UpdateInput failed: returned values are invalid" << std::endl;
					return;
				}

				// Send input to server
				Packets::PlayerMovement movementPacket;
				movementPacket.inputTime = m_server->EstimateServerTime();
				movementPacket.direction = movement;
				movementPacket.rotation = rotation;

				m_server->SendPacket(movementPacket);
			}
			else
				std::cerr << "UpdateInput failed: " << m_controlScript.GetLastError() << std::endl;
		}

		/*if (m_window.HasFocus())
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
			if (m_isCurrentlyRotating)
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
		*/
	}

	void SpaceshipController::PushToLua(const Nz::WindowEvent::MouseButtonEvent &event)
	{
		m_controlScript.PushTable(0, 3);
		{
			const char* buttonName;
			switch (event.button)
			{
				case Nz::Mouse::Left:
					buttonName = "Left";
					break;

				case Nz::Mouse::Middle:
					buttonName = "Middle";
					break;

				case Nz::Mouse::Right:
					buttonName = "Right";
					break;

				case Nz::Mouse::XButton1:
					buttonName = "XButton1";
					break;

				case Nz::Mouse::XButton2:
					buttonName = "XButton2";
					break;

				default:
					buttonName = "Unknown";
					break;
			}

			m_controlScript.PushField("button", std::string(buttonName));
			m_controlScript.PushField("x", event.x);
			m_controlScript.PushField("y", event.y);
		}
	}

	void SpaceshipController::PushToLua(const Nz::WindowEvent::KeyEvent &event)
	{
		m_controlScript.PushTable(0, 6);
		{
			m_controlScript.PushField("key", Nz::Keyboard::GetKeyName(event.code));
			m_controlScript.PushField("alt", event.alt);
			m_controlScript.PushField("control", event.control);
			m_controlScript.PushField("repeated", event.repeated);
			m_controlScript.PushField("shift", event.shift);
			m_controlScript.PushField("system", event.system);
		}
	}
}
