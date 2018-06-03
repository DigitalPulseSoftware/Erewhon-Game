// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/SpaceshipOverviewController.hpp>
#include <Nazara/Platform/Keyboard.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <Shared/Utils.hpp>
#include <Client/MatchChatbox.hpp>
#include <Client/ServerMatchEntities.hpp>

namespace ewn
{
	SpaceshipOverviewController::SpaceshipOverviewController(Nz::RenderWindow& window, const Ndk::EntityHandle& camera, MatchChatbox& chatbox, ServerMatchEntities& entities, Ndk::WorldHandle world) :
	FreeFlightController(window, camera, chatbox),
	m_entities(entities),
	m_spaceshipSelection(false)
	{
		m_onKeyPressed.Connect(m_window.GetEventHandler().OnKeyPressed, this, &SpaceshipOverviewController::OnKeyPressed);

		m_hoveredEntity = world->CreateEntity();
		m_hoveredEntity->AddComponent<Ndk::NodeComponent>().SetScale(2.f);

		// Ensure ray is always valid
		UpdateRay(Nz::Mouse::GetPosition(m_window));
	}

	SpaceshipOverviewController::~SpaceshipOverviewController() = default;

	void SpaceshipOverviewController::Update(float elapsedTime)
	{
		FreeFlightController::Update(elapsedTime);

		if (m_spaceshipSelection)
		{
			m_rayUpdateTimer -= elapsedTime;
			if (m_rayUpdateTimer < 0.f)
			{
				float closestHitDistance = std::numeric_limits<float>::infinity();
				std::size_t closestHit = std::numeric_limits<std::size_t>::max();

				std::size_t entityCount = m_entities.GetServerEntityCount();
				for (std::size_t i = 0; i < entityCount; ++i)
				{
					if (!m_entities.IsServerEntityValid(i))
						continue;

					const Ndk::EntityHandle& entity = m_entities.GetServerEntity(i).entity;
					if (!entity || !entity->HasComponent<Ndk::GraphicsComponent>())
						continue;
					
					auto& entityGfx = entity->GetComponent<Ndk::GraphicsComponent>();

					const Nz::Boxf& aabb = entityGfx.GetBoundingVolume().aabb;

					float hitDistance;
					if (m_ray.Intersect(aabb, &hitDistance))
					{
						if (hitDistance < closestHitDistance)
						{
							closestHitDistance = hitDistance;
							closestHit = i;
						}
					}
				}

				if (closestHit != m_hoveredEntityId)
				{
					m_hoveredEntityId = closestHit;
					std::cout << "Mouse is over " << m_hoveredEntityId << std::endl;

					if (m_hoveredEntityId != std::numeric_limits<std::size_t>::max())
					{
						const Ndk::EntityHandle& entity = m_entities.GetServerEntity(m_hoveredEntityId).entity;

						m_hoveredEntity->GetComponent<Ndk::NodeComponent>().SetParent(entity);
						m_hoveredEntity->GetComponent<Ndk::NodeComponent>().SetScale(2.f);
						m_hoveredEntity->AddComponent(entity->GetComponent<Ndk::GraphicsComponent>().Clone());
					}
					else
						m_hoveredEntity->RemoveComponent<Ndk::GraphicsComponent>();
				}

				m_rayUpdateTimer = 1 / 10.f;
			}
		}
	}

	void SpaceshipOverviewController::OnKeyPressed(const Nz::EventHandler* /*eventHandler*/, const Nz::WindowEvent::KeyEvent& event)
	{
		if (event.code == Nz::Keyboard::C)
		{
			m_spaceshipSelection = !m_spaceshipSelection;

			if (m_spaceshipSelection)
			{
				m_hoveredEntityId = std::numeric_limits<std::size_t>::max();
				m_rayUpdateTimer = 0.f;
				m_window.SetCursor(Nz::SystemCursor_Default);
			}
			else
				m_window.SetCursor(Nz::SystemCursor_None);
		}
	}

	void SpaceshipOverviewController::OnMouseMoved(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::MouseMoveEvent& event)
	{
		if (!m_spaceshipSelection)
		{
			FreeFlightController::OnMouseMoved(eventHandler, event);
			return;
		}

		UpdateRay({ int(event.x), int(event.y) });
	}

	void SpaceshipOverviewController::UpdateRay(Nz::Vector2i mousePos)
	{
		auto& cameraComp = m_camera->GetComponent<Ndk::CameraComponent>();

		Nz::Vector2f fMousePos = Nz::Vector2f(mousePos);
		Nz::Vector3f nearPoint = cameraComp.Unproject(Nz::Vector3f(fMousePos.x, fMousePos.y, 0.f));
		Nz::Vector3f farPoint = cameraComp.Unproject(Nz::Vector3f(fMousePos.x, fMousePos.y, 1.f));

		Nz::Vector3f direction = farPoint - nearPoint;
		direction.Normalize();

		m_ray.Set(nearPoint, direction);
		m_rayUpdateTimer = 0.f;
	}
}
