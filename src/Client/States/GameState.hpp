// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_GAMESTATE_HPP
#define EREWHON_CLIENT_STATES_GAMESTATE_HPP

#include <Client/States/StateData.hpp>
#include <Nazara/Utility/Node.hpp>
#include <NDK/Entity.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>

namespace ewn
{
	class GameState final : public Ndk::State
	{
		public:
			inline GameState(StateData& stateData);
			~GameState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			StateData& m_stateData;
			Ndk::EntityHandle m_earthEntity;
			Ndk::EntityHandle m_spaceshipEntity;
			Ndk::EntityHandle m_spaceship2Entity;
			Nz::Node m_spaceshipMovementNode;
			Nz::Vector2i m_rotationCursorOrigin;
			Nz::Vector2i m_rotationCursorPosition;
			Nz::Vector2f m_rotationDirection;
			Nz::Vector3f m_spaceshipRotation;
			Nz::Vector3f m_spaceshipSpeed;
			bool m_isCurrentlyRotating;
	};
}

#include <Client/States/GameState.inl>

#endif // EREWHON_CLIENT_STATES_GAMESTATE_HPP
