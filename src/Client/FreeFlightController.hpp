// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_FREEFLIGHTCONTROLLER_HPP
#define EREWHON_CLIENT_FREEFLIGHTCONTROLLER_HPP

#include <Nazara/Renderer/RenderWindow.hpp>
#include <NDK/Entity.hpp>

namespace ewn
{
	class MatchChatbox;

	class FreeFlightController
	{
		public:
			FreeFlightController(Nz::RenderWindow& window, const Ndk::EntityHandle& camera, MatchChatbox& chatbox);
			FreeFlightController(const FreeFlightController&) = delete;
			FreeFlightController(FreeFlightController&&) = delete;
			virtual ~FreeFlightController();

			void Update(float elapsedTime);

			FreeFlightController& operator=(const FreeFlightController&) = delete;
			FreeFlightController& operator=(FreeFlightController&&) = delete;

		protected:
			virtual void OnMouseMoved(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::MouseMoveEvent& event);

			Nz::RenderWindow& m_window;
			MatchChatbox& m_chatbox;
			Ndk::EntityHandle m_camera;

		private:
			NazaraSlot(Nz::EventHandler, OnMouseMoved, m_onMouseMovedSlot);

			Nz::EulerAnglesf m_camAngles;
			Nz::Vector3f m_targetPosition;
	};
}

#include <Client/FreeFlightController.inl>

#endif // EREWHON_CLIENT_FREEFLIGHTCONTROLLER_HPP
