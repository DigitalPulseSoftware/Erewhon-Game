// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_SPACESHIPOVERVIEWCONTROLLER_HPP
#define EREWHON_CLIENT_SPACESHIPOVERVIEWCONTROLLER_HPP

#include <Nazara/Math/Ray.hpp>
#include <NDK/EntityOwner.hpp>
#include <NDK/World.hpp>
#include <Client/FreeFlightController.hpp>

namespace ewn
{
	class ServerMatchEntities;

	class SpaceshipOverviewController : public FreeFlightController
	{
		public:
			SpaceshipOverviewController(Nz::RenderWindow& window, const Ndk::EntityHandle& camera, MatchChatbox& chatbox, ServerMatchEntities& entities, Ndk::WorldHandle world);
			SpaceshipOverviewController(const SpaceshipOverviewController&) = delete;
			SpaceshipOverviewController(SpaceshipOverviewController&&) = delete;

			~SpaceshipOverviewController();

			void Update(float elapsedTime);

			SpaceshipOverviewController& operator=(const SpaceshipOverviewController&) = delete;
			SpaceshipOverviewController& operator=(SpaceshipOverviewController&&) = delete;

		private:
			void OnKeyPressed(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::KeyEvent& event);
			void OnMouseMoved(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::MouseMoveEvent& event) override;
			void UpdateRay(Nz::Vector2i mousePos);
	
			NazaraSlot(Nz::EventHandler, OnKeyPressed, m_onKeyPressed);

			ServerMatchEntities& m_entities;
			Ndk::EntityOwner m_hoveredEntity;
			Nz::Rayf m_ray;
			std::size_t m_hoveredEntityId;
			bool m_spaceshipSelection;
			float m_rayUpdateTimer;
	};
}

#include <Client/SpaceshipOverviewController.inl>

#endif // EREWHON_CLIENT_FREEFLIGHTCONTROLLER_HPP
