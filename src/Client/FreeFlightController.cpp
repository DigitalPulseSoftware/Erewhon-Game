// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/FreeFlightController.hpp>
#include <Nazara/Platform/Keyboard.hpp>
#include <Shared/Utils.hpp>
#include <Client/MatchChatbox.hpp>
#include <NDK/Components/NodeComponent.hpp>

namespace ewn
{
	FreeFlightController::FreeFlightController(Nz::RenderWindow& window, const Ndk::EntityHandle& camera, MatchChatbox& chatbox) :
	m_window(window),
	m_chatbox(chatbox),
	m_camera(camera)
	{
		Nz::Vector2ui size = m_window.GetSize();
		Nz::Mouse::SetPosition(size.x / 2, size.y / 2, m_window);

		m_onMouseMovedSlot.Connect(m_window.GetEventHandler().OnMouseMoved, this, &FreeFlightController::OnMouseMoved);
		m_window.SetCursor(Nz::SystemCursor_None);

		auto& cameraNode = m_camera->GetComponent<Ndk::NodeComponent>();

		m_camAngles = cameraNode.GetRotation().ToEulerAngles();
		m_camAngles.roll = 0.f;

		m_targetPosition = cameraNode.GetPosition();
	}

	FreeFlightController::~FreeFlightController()
	{
		m_window.SetCursor(Nz::SystemCursor_Default);
	}

	void FreeFlightController::Update(float elapsedTime)
	{
		constexpr float speed = 100.f;

		auto& cameraNode = m_camera->GetComponent<Ndk::NodeComponent>();
		if (m_window.HasFocus() && !m_chatbox.IsTyping())
		{
			float cameraSpeed = speed * elapsedTime;

			if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Space))
				cameraSpeed *= 5.f;

			if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Up) || Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Z))
				m_targetPosition += cameraNode.GetForward() * cameraSpeed;

			if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Down) || Nz::Keyboard::IsKeyPressed(Nz::Keyboard::S))
				m_targetPosition += cameraNode.GetBackward() * cameraSpeed;

			if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Left) || Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Q))
				m_targetPosition += cameraNode.GetLeft() * cameraSpeed;

			if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Right) || Nz::Keyboard::IsKeyPressed(Nz::Keyboard::D))
				m_targetPosition += cameraNode.GetRight() * cameraSpeed;

			if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::LShift) || Nz::Keyboard::IsKeyPressed(Nz::Keyboard::RShift))
				m_targetPosition += Nz::Vector3f::Up() * cameraSpeed;

			if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::LControl) || Nz::Keyboard::IsKeyPressed(Nz::Keyboard::RControl))
				m_targetPosition += Nz::Vector3f::Down() * cameraSpeed;
		}

		cameraNode.SetPosition(DampenedString(cameraNode.GetPosition(), m_targetPosition, elapsedTime), Nz::CoordSys_Global);
	}

	void FreeFlightController::OnMouseMoved(const Nz::EventHandler* /*eventHandler*/, const Nz::WindowEvent::MouseMoveEvent& event)
	{
		float sensitivity = 0.3f; // Mouse sensitivity

		m_camAngles.yaw = Nz::NormalizeAngle(m_camAngles.yaw - event.deltaX*sensitivity);
		m_camAngles.pitch = Nz::Clamp(m_camAngles.pitch - event.deltaY*sensitivity, -89.f, 89.f);

		// Apply Euler angles to our quaternion camera
		auto& cameraNode = m_camera->GetComponent<Ndk::NodeComponent>();
		cameraNode.SetRotation(m_camAngles);

		Nz::Vector2ui size = m_window.GetSize();
		Nz::Mouse::SetPosition(size.x / 2, size.y / 2, m_window);
	}
}
