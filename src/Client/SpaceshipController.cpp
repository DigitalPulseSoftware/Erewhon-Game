// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/SpaceshipController.hpp>
#include <Nazara/Core/CallOnExit.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent3D.hpp>
#include <NDK/LuaAPI.hpp>
#include <Client/ClientApplication.hpp>
#include <Client/MatchChatbox.hpp>
#include <Client/ServerMatchEntities.hpp>
#include <string>

namespace ewn
{
	SpaceshipController::SpaceshipController(ClientApplication* app, ServerConnection* server, Nz::RenderWindow& window, Ndk::World& world2D, MatchChatbox& chatbox, ServerMatchEntities& entities, const Ndk::EntityHandle& camera, const Ndk::EntityHandle& spaceship) :
	m_app(app),
	m_chatbox(chatbox),
	m_entities(entities),
	m_server(server),
	m_world2D(world2D),
	m_window(window),
	m_camera(camera),
	m_spaceship(spaceship),
	m_lastShootTime(0),
	m_executeScript(false),
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

		// Load client script
		LoadScript();
	}

	SpaceshipController::~SpaceshipController()
	{
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

		if (m_executeScript)
		{
			if (m_controlScript.GetGlobal("OnUpdate") == Nz::LuaType_Function)
			{
				auto& spaceshipNode = m_spaceship->GetComponent<Ndk::NodeComponent>();
				Nz::Vector3f position = spaceshipNode.GetPosition();
				Nz::Quaternionf rotation = spaceshipNode.GetRotation();

				m_controlScript.PushTable(0, 3);
				m_controlScript.PushField("x", position.x);
				m_controlScript.PushField("y", position.y);
				m_controlScript.PushField("z", position.z);

				m_controlScript.PushTable(0, 4);
				m_controlScript.PushField("w", rotation.w);
				m_controlScript.PushField("x", rotation.x);
				m_controlScript.PushField("y", rotation.y);
				m_controlScript.PushField("z", rotation.z);

				if (!m_controlScript.Call(2))
					std::cerr << "OnUpdate failed: " << m_controlScript.GetLastError() << std::endl;
			}
		}
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

		if (m_executeScript)
		{
			if (m_controlScript.GetGlobal("OnIntegrityUpdate") == Nz::LuaType_Function)
			{
				m_controlScript.Push(integrityPct);

				if (!m_controlScript.Call(1))
					std::cerr << "OnIntegrityUpdate failed: " << m_controlScript.GetLastError() << std::endl;
			}
		}
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
		if (m_executeScript)
		{
			if (m_controlScript.GetGlobal("OnMouseMoved") == Nz::LuaType_Function)
			{
				PushToLua(event);

				if (!m_controlScript.Call(1))
					std::cerr << "OnMouseMoved failed: " << m_controlScript.GetLastError() << std::endl;
			}
		}
	}

	void SpaceshipController::OnRenderTargetSizeChange(const Nz::RenderTarget* renderTarget)
	{
		if (m_executeScript)
		{
			if (m_controlScript.GetGlobal("OnWindowSizeChanged") == Nz::LuaType_Function)
			{
				m_controlScript.PushTable(0, 2);
					m_controlScript.PushField("width", renderTarget->GetSize().x);
					m_controlScript.PushField("height", renderTarget->GetSize().y);

				if (!m_controlScript.Call(1))
					std::cerr << "OnWindowSizeChanged failed: " << m_controlScript.GetLastError() << std::endl;
			}
		}

		m_healthBarEntity->GetComponent<Ndk::NodeComponent>().SetPosition({ renderTarget->GetSize().x - 300.f, renderTarget->GetSize().y - 70.f, 0.f });
	}

	void SpaceshipController::LoadScript()
	{
		m_sprites.clear();
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
				m_window.SetCursor(Nz::SystemCursor_Default);
			else
				m_window.SetCursor(Nz::SystemCursor_None);

			return 0;
		});
		m_controlScript.SetGlobal("ShowCursor");

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			Shoot();
			return 0;
		});
		m_controlScript.SetGlobal("Shoot");

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			Nz::Vector2ui windowCenter = m_window.GetSize() / 2;
			Nz::Mouse::SetPosition(windowCenter.x, windowCenter.y, m_window);

			return 0;
		});
		m_controlScript.SetGlobal("RecenterMouse");

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			int argIndex = 1;
			Nz::Vector3f position = state.Check<Nz::Vector3f>(&argIndex);

			auto& cameraComponent = m_camera->GetComponent<Ndk::CameraComponent>();

			Nz::Vector4f worldPosition(position, 1.f);
			worldPosition = cameraComponent.GetViewMatrix() * worldPosition;
			worldPosition = cameraComponent.GetProjectionMatrix() * worldPosition;
			worldPosition /= worldPosition.w;

			Nz::Vector3f screenPosition(worldPosition.x * 0.5f + 0.5f, -worldPosition.y * 0.5f + 0.5f, worldPosition.z * 0.5f + 0.5f);
			screenPosition.x *= m_window.GetSize().x;
			screenPosition.y *= m_window.GetSize().y;

			state.PushTable(0, 2);
				state.PushField("x", screenPosition.x);
				state.PushField("y", screenPosition.y);

			return 1;
		});
		m_controlScript.SetGlobal("Project");

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			int argIndex = 1;
			Nz::Vector3f position = state.Check<Nz::Vector3f>(&argIndex);
			Nz::Quaternionf rotation = state.Check<Nz::Quaternionf>(&argIndex);

			auto& cameraNode = m_camera->GetComponent<Ndk::NodeComponent>();
			cameraNode.SetPosition(position);
			cameraNode.SetRotation(rotation);

			return 0;
		});
		m_controlScript.SetGlobal("UpdateCamera");

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			auto& spaceshipPhys = m_spaceship->GetComponent<Ndk::PhysicsComponent3D>();

			Nz::Vector3f linearVelocity = spaceshipPhys.GetLinearVelocity();

			state.PushTable(0, 3);
				state.PushField("x", linearVelocity.x);
				state.PushField("y", linearVelocity.y);
				state.PushField("z", linearVelocity.z);

			return 1;
		});
		m_controlScript.SetGlobal("GetSpaceshipLinearVelocity");

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			auto& spaceshipPhys = m_spaceship->GetComponent<Ndk::PhysicsComponent3D>();

			Nz::Vector3f angularVelocity = spaceshipPhys.GetAngularVelocity();

			state.PushTable(0, 3);
				state.PushField("x", angularVelocity.x);
				state.PushField("y", angularVelocity.y);
				state.PushField("z", angularVelocity.z);

			return 1;
		});
		m_controlScript.SetGlobal("GetSpaceshipAngularVelocity");

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			state.Push(m_chatbox.IsTyping());
			return 1;
		});
		m_controlScript.SetGlobal("IsChatboxActive");

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			m_chatbox.PrintMessage(state.CheckString(1));
			return 0;
		});
		m_controlScript.SetGlobal("PrintChatbox");

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			m_sprites.clear();
			return 0;
		});
		m_controlScript.SetGlobal("ClearSprites");

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			int argIndex = 1;
			Nz::String texture = state.Check<Nz::String>(&argIndex);
			Nz::Vector2f position = state.Check<Nz::Vector2f>(&argIndex);
			float rotation = state.Check<float>(&argIndex);
			Nz::Vector2f size = state.Check<Nz::Vector2f>(&argIndex);
			int renderOrder = state.Check<int>(&argIndex, 0);

			std::size_t newSpritePos;
			for (newSpritePos = 0; newSpritePos < m_sprites.size(); ++newSpritePos)
			{
				if (!m_sprites[newSpritePos].isValid)
					break;
			}

			if (newSpritePos >= m_sprites.size())
				m_sprites.resize(newSpritePos + 1);

			Sprite& spriteData = m_sprites[newSpritePos];
			spriteData.isValid = true;

			Nz::MaterialRef spriteMat = Nz::Material::New("Translucent2D");
			if (!texture.IsEmpty())
				spriteMat->SetDiffuseMap(texture);

			spriteData.sprite = Nz::Sprite::New();
			spriteData.sprite->SetMaterial(spriteMat);
			spriteData.sprite->SetSize(size);
			spriteData.sprite->SetOrigin(spriteData.sprite->GetSize() / 2.f);

			spriteData.entity = m_world2D.CreateEntity();

			spriteData.entity->AddComponent<Ndk::GraphicsComponent>().Attach(spriteData.sprite, renderOrder);

			auto& spriteNode = spriteData.entity->AddComponent<Ndk::NodeComponent>();
			spriteNode.SetPosition(position);
			spriteNode.SetRotation(Nz::EulerAnglesf(0.f, 0.f, rotation));

			state.Push(newSpritePos);
			return 1;
		});
		m_controlScript.SetGlobal("AddSprite");

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			int argIndex = 1;
			std::size_t spriteId = state.Check<std::size_t>(&argIndex);
			state.ArgCheck(spriteId < m_sprites.size() && m_sprites[spriteId].isValid, 1, "Invalid sprite id");

			assert(spriteId < m_sprites.size());
			m_sprites[spriteId].entity->Kill();
			m_sprites[spriteId].sprite.Reset();
			m_sprites[spriteId].isValid = false;

			return 0;
		});
		m_controlScript.SetGlobal("DeleteSprite");

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			int argIndex = 1;
			std::size_t spriteId = state.Check<std::size_t>(&argIndex);
			state.ArgCheck(spriteId < m_sprites.size() && m_sprites[spriteId].isValid, 1, "Invalid sprite id");

			Nz::Vector2f position = state.Check<Nz::Vector2f>(&argIndex);
			float rotation = state.Check<float>(&argIndex);

			Sprite& spriteData = m_sprites[spriteId];

			auto& spriteNode = spriteData.entity->AddComponent<Ndk::NodeComponent>();
			spriteNode.SetPosition(position);
			spriteNode.SetRotation(Nz::EulerAnglesf(0.f, 0.f, rotation));

			return 0;
		});
		m_controlScript.SetGlobal("UpdateSpritePosition");

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			int argIndex = 1;
			std::size_t spriteId = state.Check<std::size_t>(&argIndex);
			state.ArgCheck(spriteId < m_sprites.size() && m_sprites[spriteId].isValid, 1, "Invalid sprite id");

			Nz::Vector2f size = state.Check<Nz::Vector2f>(&argIndex);

			Sprite& spriteData = m_sprites[spriteId];
			spriteData.sprite->SetSize(size);
			spriteData.sprite->SetOrigin(size / 2.f);

			return 0;
		});
		m_controlScript.SetGlobal("UpdateSpriteSize");

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			int argIndex = 1;
			std::size_t spriteId = state.Check<std::size_t>(&argIndex);
			state.ArgCheck(spriteId < m_sprites.size() && m_sprites[spriteId].isValid, 1, "Invalid sprite id");

			Nz::Color color = state.Check<Nz::Color>(&argIndex);

			Sprite& spriteData = m_sprites[spriteId];
			spriteData.sprite->SetColor(color);

			return 0;
		});
		m_controlScript.SetGlobal("UpdateSpriteColor");

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			int argIndex = 1;
			std::size_t spriteId = state.Check<std::size_t>(&argIndex);
			bool enable = state.Check<bool>(&argIndex);
			state.ArgCheck(spriteId < m_sprites.size() && m_sprites[spriteId].isValid, 1, "Invalid sprite id");

			Sprite& spriteData = m_sprites[spriteId];
			spriteData.entity->Enable(enable);

			return 0;
		});
		m_controlScript.SetGlobal("ShowSprite");

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			int argIndex = 1;
			bool enable = state.Check<bool>(&argIndex);

			m_healthBarEntity->Enable(enable);
			return 0;
		});
		m_controlScript.SetGlobal("ShowHealthbar");

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			m_controlScript.PushTable(0, 2);
				m_controlScript.PushField("width", m_window.GetSize().x);
				m_controlScript.PushField("height", m_window.GetSize().y);

			return 1;
		});
		m_controlScript.SetGlobal("GetScreenSize");

		m_controlScript.PushFunction([this](Nz::LuaState& state) -> int
		{
			std::size_t entityCount = m_entities.GetServerEntityCount();

			m_controlScript.PushTable(0, entityCount);

			for (std::size_t i = 0; i < entityCount; ++i)
			{
				if (!m_entities.IsServerEntityValid(i))
					continue;

				const auto& entityData = m_entities.GetServerEntity(i);

				const Ndk::EntityHandle& entity = entityData.entity;
				if (entity == m_spaceship)
					continue;

				auto& entityNode = entity->GetComponent<Ndk::NodeComponent>();
				auto& entityPhys = entity->GetComponent<Ndk::PhysicsComponent3D>();

				m_controlScript.PushInteger(entity->GetId());
				m_controlScript.PushTable(0, 5);
				{
					m_controlScript.PushField("name", entityData.name);

					// Kill me
					switch (entityData.type)
					{
						case ServerMatchEntities::Type::Ball:
							m_controlScript.PushField("type", std::string("ball"));
							break;
						case ServerMatchEntities::Type::Earth:
							m_controlScript.PushField("type", std::string("earth"));
							break;
						case ServerMatchEntities::Type::Projectile:
							m_controlScript.PushField("type", std::string("projectile"));
							break;
						case ServerMatchEntities::Type::Spaceship:
							m_controlScript.PushField("type", std::string("spaceship"));
							break;
						default:
							m_controlScript.PushField("type", std::string("bug"));
							break;
					}

					m_controlScript.PushTable(0, 3);
					{
						Nz::Vector3f position = entityNode.GetPosition();
						m_controlScript.PushField("x", position.x);
						m_controlScript.PushField("y", position.y);
						m_controlScript.PushField("z", position.z);
					}
					m_controlScript.SetField("position");

					m_controlScript.PushTable(0, 3);
					{
						Nz::Quaternionf rotation = entityNode.GetRotation();
						m_controlScript.PushField("w", rotation.x);
						m_controlScript.PushField("x", rotation.x);
						m_controlScript.PushField("y", rotation.y);
						m_controlScript.PushField("z", rotation.z);
					}
					m_controlScript.SetField("rotation");

					m_controlScript.PushTable(0, 3);
					{
						Nz::Vector3f velocity = entityPhys.GetLinearVelocity();
						m_controlScript.PushField("x", velocity.x);
						m_controlScript.PushField("y", velocity.y);
						m_controlScript.PushField("z", velocity.z);
					}
					m_controlScript.SetField("linearVelocity");

					m_controlScript.PushTable(0, 3);
					{
						Nz::Vector3f velocity = entityPhys.GetAngularVelocity();
						m_controlScript.PushField("x", velocity.x);
						m_controlScript.PushField("y", velocity.y);
						m_controlScript.PushField("z", velocity.z);
					}
					m_controlScript.SetField("angularVelocity");
				}

				m_controlScript.SetTable();
			}

			return 1;
		});
		m_controlScript.SetGlobal("ScanEntities");

		if (m_executeScript)
		{
			if (m_controlScript.GetGlobal("Init") == Nz::LuaType_Function)
			{
				if (!m_controlScript.Call(0))
					std::cerr << "Init failed: " << m_controlScript.GetLastError() << std::endl;
			}
		}
	}

	void SpaceshipController::LoadSprites(Ndk::World& world2D)
	{
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
			if (m_controlScript.Call(1))
			{
				// Use some RRID
				Nz::CallOnExit resetLuaStack([&]()
				{
					m_controlScript.Pop(m_controlScript.GetStackTop());
				});

				Nz::Vector3f movement;
				Nz::Vector3f rotation;

				try
				{
					int index = 1;
					movement = m_controlScript.Check<Nz::Vector3f>(&index);
					rotation = m_controlScript.Check<Nz::Vector3f>(&index);
				}
				catch (const std::exception&)
				{
					std::cerr << "UpdateInput failed: returned values are invalid:\n";
					std::cerr << m_controlScript.DumpStack() << std::endl;
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
	}

	void SpaceshipController::PushToLua(const Nz::WindowEvent::KeyEvent& event)
	{
		m_controlScript.PushTable(0, 6);
		{
			std::string keyName;
#define HandleKey(KeyName) case Nz::Keyboard::##KeyName : keyName = #KeyName ; break;
			switch (event.code)
			{
				HandleKey(Undefined)

				// Lettres
				HandleKey(A)
				HandleKey(B)
				HandleKey(C)
				HandleKey(D)
				HandleKey(E)
				HandleKey(F)
				HandleKey(G)
				HandleKey(H)
				HandleKey(I)
				HandleKey(J)
				HandleKey(K)
				HandleKey(L)
				HandleKey(M)
				HandleKey(N)
				HandleKey(O)
				HandleKey(P)
				HandleKey(Q)
				HandleKey(R)
				HandleKey(S)
				HandleKey(T)
				HandleKey(U)
				HandleKey(V)
				HandleKey(W)
				HandleKey(X)
				HandleKey(Y)
				HandleKey(Z)

				// Functional keys
				HandleKey(F1)
				HandleKey(F2)
				HandleKey(F3)
				HandleKey(F4)
				HandleKey(F5)
				HandleKey(F6)
				HandleKey(F7)
				HandleKey(F8)
				HandleKey(F9)
				HandleKey(F10)
				HandleKey(F11)
				HandleKey(F12)
				HandleKey(F13)
				HandleKey(F14)
				HandleKey(F15)

				// Directional keys
				HandleKey(Down)
				HandleKey(Left)
				HandleKey(Right)
				HandleKey(Up)

				// Numerical pad
				HandleKey(Add)
				HandleKey(Decimal)
				HandleKey(Divide)
				HandleKey(Multiply)
				HandleKey(Numpad0)
				HandleKey(Numpad1)
				HandleKey(Numpad2)
				HandleKey(Numpad3)
				HandleKey(Numpad4)
				HandleKey(Numpad5)
				HandleKey(Numpad6)
				HandleKey(Numpad7)
				HandleKey(Numpad8)
				HandleKey(Numpad9)
				HandleKey(Subtract)

				// Various
				HandleKey(Backslash)
				HandleKey(Backspace)
				HandleKey(Clear)
				HandleKey(Comma)
				HandleKey(Dash)
				HandleKey(Delete)
				HandleKey(End)
				HandleKey(Equal)
				HandleKey(Escape)
				HandleKey(Home)
				HandleKey(Insert)
				HandleKey(LAlt)
				HandleKey(LBracket)
				HandleKey(LControl)
				HandleKey(LShift)
				HandleKey(LSystem)
				HandleKey(Num0)
				HandleKey(Num1)
				HandleKey(Num2)
				HandleKey(Num3)
				HandleKey(Num4)
				HandleKey(Num5)
				HandleKey(Num6)
				HandleKey(Num7)
				HandleKey(Num8)
				HandleKey(Num9)
				HandleKey(PageDown)
				HandleKey(PageUp)
				HandleKey(Pause)
				HandleKey(Period)
				HandleKey(Print)
				HandleKey(PrintScreen)
				HandleKey(Quote)
				HandleKey(RAlt)
				HandleKey(RBracket)
				HandleKey(RControl)
				HandleKey(Return)
				HandleKey(RShift)
				HandleKey(RSystem)
				HandleKey(Semicolon)
				HandleKey(Slash)
				HandleKey(Space)
				HandleKey(Tab)
				HandleKey(Tilde)

				// Navigator keys
				HandleKey(Browser_Back)
				HandleKey(Browser_Favorites)
				HandleKey(Browser_Forward)
				HandleKey(Browser_Home)
				HandleKey(Browser_Refresh)
				HandleKey(Browser_Search)
				HandleKey(Browser_Stop)

				// Lecture control keys
				HandleKey(Media_Next)
				HandleKey(Media_Play)
				HandleKey(Media_Previous)
				HandleKey(Media_Stop)

				// Volume control keys
				HandleKey(Volume_Down)
				HandleKey(Volume_Mute)
				HandleKey(Volume_Up)

				// Locking keys
				HandleKey(CapsLock)
				HandleKey(NumLock)
				HandleKey(ScrollLock)

				default: keyName = "Unknown"; break;
			}
#undef HandleKey

			m_controlScript.PushField("key", keyName);
			m_controlScript.PushField("alt", event.alt);
			m_controlScript.PushField("control", event.control);
			m_controlScript.PushField("repeated", event.repeated);
			m_controlScript.PushField("shift", event.shift);
			m_controlScript.PushField("system", event.system);
		}
	}

	void SpaceshipController::PushToLua(const Nz::WindowEvent::MouseButtonEvent& event)
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

	void SpaceshipController::PushToLua(const Nz::WindowEvent::MouseMoveEvent& event)
	{
		m_controlScript.PushTable(0, 4);
			m_controlScript.PushField("deltaX", event.deltaX);
			m_controlScript.PushField("deltaY", event.deltaY);
			m_controlScript.PushField("x", event.x);
			m_controlScript.PushField("y", event.y);
	}
}
