// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_GAMESTATE_HPP
#define EREWHON_CLIENT_STATES_GAMESTATE_HPP

#include <Client/States/StateData.hpp>
#include <Nazara/Core/Clock.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Network/UdpSocket.hpp>
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
			struct ClientInput
			{
				Nz::UInt8 inputId;
				Nz::UInt64 inputTime;
				Nz::Quaternionf rotation;
				Nz::Vector3f position;
				Nz::Vector3f velocity;
				Nz::Vector3f angularVelocity;
			};

			struct ServerEntity
			{
				Ndk::EntityHandle debugGhostEntity;
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

			void OnArenaState(ServerConnection* server, const Packets::ArenaState& arenaState);
			void OnChatMessage(ServerConnection* server, const Packets::ChatMessage& chatMessage);
			void OnControlSpaceship(ServerConnection* server, const Packets::ControlSpaceship& controlPacket);
			void OnCreateSpaceship(ServerConnection* server, const Packets::CreateSpaceship& createPacket);
			void OnDeleteSpaceship(ServerConnection* server, const Packets::DeleteSpaceship& deletePacket);
			void OnKeyPressed(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::KeyEvent& event);

			void UpdateInput(float elapsedTime);

			NazaraSlot(ServerConnection, OnArenaState, m_onArenaStateSlot);
			NazaraSlot(ServerConnection, OnChatMessage, m_onChatMessageSlot);
			NazaraSlot(ServerConnection, OnControlSpaceship, m_onControlSpaceshipSlot);
			NazaraSlot(ServerConnection, OnCreateSpaceship, m_onCreateSpaceshipSlot);
			NazaraSlot(ServerConnection, OnDeleteSpaceship, m_onDeleteSpaceshipSlot);
			NazaraSlot(Nz::EventHandler, OnKeyPressed, m_onKeyPressedSlot);
			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			StateData& m_stateData;
			Ndk::EntityHandle m_cursorEntity;
			Ndk::EntityHandle m_earthEntity;
			Ndk::EntityHandle m_debugTemplateEntity;
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
			Nz::UdpSocket m_debugStateSocket;
			Nz::UInt64 m_serverTimeDelta;
			std::size_t m_controlledEntity;
			std::vector<ClientInput> m_predictedInputs;
			std::vector<ServerEntity> m_serverEntities;
			std::vector<Nz::String> m_chatLines;
			bool m_isCurrentlyRotating;
			float m_interpolationFactor;
	};
}

#include <Client/States/GameState.inl>

#endif // EREWHON_CLIENT_STATES_GAMESTATE_HPP
