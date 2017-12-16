// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_SPACESHIPCONTROLLER_HPP
#define EREWHON_CLIENT_SPACESHIPCONTROLLER_HPP

#include <Nazara/Audio/Sound.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Lua/LuaInstance.hpp>
#include <Nazara/Renderer/RenderWindow.hpp>
#include <Nazara/Utility/Node.hpp>
#include <NDK/Entity.hpp>
#include <NDK/EntityOwner.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Client/ServerConnection.hpp>

namespace ewn
{
	class ClientApplication;

	class SpaceshipController
	{
		public:
			SpaceshipController(ClientApplication* app, ServerConnection* server, Nz::RenderWindow& window, Ndk::World& world2D, const Ndk::EntityHandle& camera, const Ndk::EntityHandle& spaceship);
			SpaceshipController(const SpaceshipController&) = delete;
			SpaceshipController(SpaceshipController&&) = delete;
			~SpaceshipController();

			void Update(float elapsedTime);

			SpaceshipController& operator=(const SpaceshipController&) = delete;
			SpaceshipController& operator=(SpaceshipController&&) = delete;

		private:
			void OnKeyPressed(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::KeyEvent& event);

			void PushToLua(const Nz::WindowEvent::KeyEvent &event);

			void OnKeyReleased(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::KeyEvent& event);
			void OnLostFocus(const Nz::EventHandler* eventHandler);
			void OnIntegrityUpdate(ServerConnection* server, const Packets::IntegrityUpdate& integrityUpdate);
			void OnMouseButtonPressed(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::MouseButtonEvent& event);

			void PushToLua(const Nz::WindowEvent::MouseButtonEvent &event);

			void OnMouseButtonReleased(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::MouseButtonEvent& event);
			void OnMouseMoved(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::MouseMoveEvent& event);
			void OnRenderTargetSizeChange(const Nz::RenderTarget* renderTarget);

			void LoadScript();
			void LoadSprites(Ndk::World& world2D);
			void Shoot();
			void UpdateInput(float elapsedTime);

			NazaraSlot(ServerConnection, OnIntegrityUpdate, m_onIntegrityUpdateSlot);
			NazaraSlot(Nz::EventHandler, OnKeyPressed, m_onKeyPressedSlot);
			NazaraSlot(Nz::EventHandler, OnKeyReleased, m_onKeyReleasedSlot);
			NazaraSlot(Nz::EventHandler, OnLostFocus, m_onLostFocusSlot);
			NazaraSlot(Nz::EventHandler, OnMouseButtonPressed, m_onMouseButtonPressedSlot);
			NazaraSlot(Nz::EventHandler, OnMouseButtonReleased, m_onMouseButtonReleasedSlot);
			NazaraSlot(Nz::EventHandler, OnMouseMoved, m_onMouseMovedSlot);
			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			ClientApplication* m_app;
			ServerConnection* m_server;
			Nz::RenderWindow& m_window;
			Ndk::EntityHandle m_camera;
			Ndk::EntityOwner m_crosshairEntity;
			Ndk::EntityOwner m_cursorEntity;
			Ndk::EntityOwner m_healthBarEntity;
			Ndk::EntityHandle m_spaceship;
			Nz::LuaInstance m_controlScript;
			Nz::Node m_cameraNode;
			Nz::SpriteRef m_cursorOrientationSprite;
			Nz::SpriteRef m_healthBarSprite;
			Nz::Sound m_shootSound;
			Nz::UInt64 m_lastShootTime;
			Nz::Vector2i m_rotationCursorOrigin;
			Nz::Vector2i m_rotationCursorPosition;
			Nz::Vector3f m_cameraRotation;
			bool m_executeScript;
			bool m_isCurrentlyRotating;
			float m_inputAccumulator;
	};
}

#include <Client/SpaceshipController.inl>

#endif // EREWHON_CLIENT_SPACESHIPCONTROLLER_HPP
