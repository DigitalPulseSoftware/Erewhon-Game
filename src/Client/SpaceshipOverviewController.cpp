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
	m_world(world),
	m_spaceshipSelection(false)
	{
		Nz::EventHandler& eventHandler = m_window.GetEventHandler();
		m_onKeyPressed.Connect(eventHandler.OnKeyPressed, this, &SpaceshipOverviewController::OnKeyPressed);
		m_onMouseButtonPressed.Connect(eventHandler.OnMouseButtonPressed, this, &SpaceshipOverviewController::OnMouseButtonPressed);

		m_rayStart = Nz::Vector3f::Zero();
		m_rayEnd = Nz::Vector3f::Forward();

		m_hoveredMaterial = Nz::Material::New();
		m_hoveredMaterial->SetFaceCulling(Nz::FaceSide_Front);

		m_hoveredEntity = m_world->CreateEntity();
		m_hoveredEntity->AddComponent<Ndk::NodeComponent>();
		m_hoveredEntity->AddComponent<Ndk::GraphicsComponent>();

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
				auto& cameraNode = m_camera->GetComponent<Ndk::NodeComponent>();

				Nz::Vector3f rayStart = cameraNode.ToGlobalPosition(m_rayStart);
				Nz::Vector3f rayEnd = cameraNode.ToGlobalPosition(m_rayEnd);

				Nz::Vector3f rayDirection = rayEnd - rayStart;
				rayDirection.Normalize();

				Nz::Rayf ray(rayStart, rayDirection);

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

					const Nz::Boxf& aabb = entityGfx.GetAABB();

					float hitDistance;
					if (ray.Intersect(aabb, &hitDistance))
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
					OnHoverEntityChange(this, closestHit);

					m_hoveredEntityId = closestHit;

					if (closestHit != NoEntity)
					{
						const Ndk::EntityHandle& entity = m_entities.GetServerEntity(m_hoveredEntityId).entity;

						auto& entityGfx = entity->GetComponent<Ndk::GraphicsComponent>();

						m_hoveredEntitySize = entityGfx.GetAABB().GetRadius();

						m_hoveredEntity->Enable();

						auto& hoveredNode = m_hoveredEntity->GetComponent<Ndk::NodeComponent>();
						hoveredNode.SetParent(entity);

						auto& hoveredGfx = m_hoveredEntity->GetComponent<Ndk::GraphicsComponent>();
						hoveredGfx.Clear();

						entity->GetComponent<Ndk::GraphicsComponent>().ForEachRenderable([&](const Nz::InstancedRenderableRef& renderable, const Nz::Matrix4f& localMatrix, int renderOrder)
						{
							if (std::unique_ptr<Nz::InstancedRenderable> clonedRenderable = renderable->Clone())
							{
								std::size_t materialCount = clonedRenderable->GetMaterialCount();
								for (std::size_t i = 0; i < materialCount; ++i)
									clonedRenderable->SetMaterial(i, m_hoveredMaterial);

								hoveredGfx.Attach(clonedRenderable.release(), localMatrix, -1);
							}
						});
					}
					else
						m_hoveredEntity->Disable();
				}

				m_rayUpdateTimer = 1 / 10.f;
			}
		}

		if (m_hoveredEntityId != NoEntity)
		{
			auto& hoveredNode = m_hoveredEntity->GetComponent<Ndk::NodeComponent>();
			auto& hoveredGfx = m_hoveredEntity->GetComponent<Ndk::GraphicsComponent>();

			auto& cameraComp = m_camera->GetComponent<Ndk::CameraComponent>();
			float distance = cameraComp.GetFrustum().GetPlane(Nz::FrustumPlane_Near).Distance(hoveredNode.GetPosition());

			float distanceFactor = distance / 3000.f;
			float sizeFactor = 40.f / m_hoveredEntitySize;

			hoveredNode.SetScale(1.f + distanceFactor * sizeFactor);
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

	void SpaceshipOverviewController::OnMouseButtonPressed(const Nz::EventHandler* /*eventHandler*/, const Nz::WindowEvent::MouseButtonEvent& event)
	{
		if (event.button == Nz::Mouse::Left)
		{
			if (m_spaceshipSelection && m_hoveredEntityId != NoEntity)
				OnEntityClick(this, m_hoveredEntityId);
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
		auto& cameraNode = m_camera->GetComponent<Ndk::NodeComponent>();

		Nz::Vector2f fMousePos = Nz::Vector2f(mousePos);
		Nz::Vector3f nearPoint = cameraComp.Unproject(Nz::Vector3f(fMousePos.x, fMousePos.y, 0.f));
		Nz::Vector3f farPoint = cameraComp.Unproject(Nz::Vector3f(fMousePos.x, fMousePos.y, 1.f));

		m_rayStart = cameraNode.ToLocalPosition(nearPoint);
		m_rayEnd = cameraNode.ToLocalPosition(farPoint);

		m_rayUpdateTimer = 0.f;
	}
}
