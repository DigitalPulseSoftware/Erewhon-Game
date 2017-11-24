// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_GAMESTATE_HPP
#define EREWHON_CLIENT_STATES_GAMESTATE_HPP

#include <Client/States/StateData.hpp>
#include <Nazara/Core/Clock.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Utility/Node.hpp>
#include <NDK/Entity.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>
#include <unordered_map>

namespace ewn
{
	class GameState final : public Ndk::State
	{
		public:
			inline GameState(StateData& stateData);
			~GameState() = default;

		private:
			struct ServerEntity
			{
				Ndk::EntityHandle shipEntity;
				Ndk::EntityHandle textEntity;
				Nz::Quaternionf oldRotation;
				Nz::Quaternionf newRotation;
				Nz::Vector3f oldPosition;
				Nz::Vector3f newPosition;
				bool isValid = false;
			};

			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			inline ServerEntity& CreateServerEntity(std::size_t id);
			inline ServerEntity& GetServerEntity(std::size_t id);

			void OnArenaState(const Packets::ArenaState& arenaState);
			void OnChatMessage(const Packets::ChatMessage& chatMessage);
			void OnControlSpaceship(const Packets::ControlSpaceship& controlPacket);
			void OnCreateSpaceship(const Packets::CreateSpaceship& createPacket);
			void OnDeleteSpaceship(const Packets::DeleteSpaceship& deletePacket);
			void OnKeyPressed(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::KeyEvent& event);

			void UpdateInput(float elapsedTime);

			NazaraSlot(ClientApplication, OnArenaState, m_onArenaStateSlot);
			NazaraSlot(ClientApplication, OnChatMessage, m_onChatMessageSlot);
			NazaraSlot(ClientApplication, OnControlSpaceship, m_onControlSpaceshipSlot);
			NazaraSlot(ClientApplication, OnCreateSpaceship, m_onCreateSpaceshipSlot);
			NazaraSlot(ClientApplication, OnDeleteSpaceship, m_onDeleteSpaceshipSlot);
			NazaraSlot(Nz::EventHandler, OnKeyPressed, m_onKeyPressedSlot);

			StateData& m_stateData;
			Ndk::EntityHandle m_cursorEntity;
			Ndk::EntityHandle m_earthEntity;
			Ndk::EntityHandle m_spaceshipTemplateEntity;
			Ndk::TextAreaWidget* m_chatBox;
			Ndk::TextAreaWidget* m_chatEnteringBox;
			Nz::Clock m_inputClock;
			Nz::Node m_spaceshipMovementNode;
			Nz::SpriteRef m_cursorOrientationSprite;
			Nz::Vector2i m_rotationCursorOrigin;
			Nz::Vector2i m_rotationCursorPosition;
			Nz::Vector2f m_rotationDirection;
			Nz::Vector3f m_spaceshipRotation;
			Nz::Vector3f m_spaceshipSpeed;
			Nz::UInt8 m_lastStateId;
			Nz::UInt64 m_serverTimeDelta;
			std::size_t m_controlledEntity;
			std::size_t m_currentInterpolationState;
			std::size_t m_stateCount;
			std::vector<ServerEntity> m_serverEntities;
			std::vector<Nz::String> m_chatLines;
			bool m_isCurrentlyRotating;
			float m_interpolationFactor;
	};
}

#include <Client/States/GameState.inl>

#endif // EREWHON_CLIENT_STATES_GAMESTATE_HPP
