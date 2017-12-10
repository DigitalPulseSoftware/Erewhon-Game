// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_GAMESTATE_HPP
#define EREWHON_CLIENT_STATES_GAMESTATE_HPP

#include <Client/ServerMatchEntities.hpp>
#include <Client/States/StateData.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Network/UdpSocket.hpp>
#include <Nazara/Utility/Node.hpp>
#include <NDK/Entity.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>
#include <optional>
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
				Nz::UInt64 inputTime;
				Nz::Vector3f movement;
				Nz::Vector3f rotation;
			};

			struct ServerSnapshot
			{
				struct StateData
				{
					Nz::Quaternionf rotation;
					Nz::Vector3f position;
					Nz::Vector3f angularVelocity;
					Nz::Vector3f linearVelocity;
				};

				Nz::UInt64 snapshotId;
				std::vector<StateData> states;
			};

			struct ServerEntity
			{
				Ndk::EntityHandle debugGhostEntity;
				Ndk::EntityHandle entity;
				Ndk::EntityHandle textEntity;
				bool isValid = false;
			};

			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			inline ServerEntity& CreateServerEntity(std::size_t id);
			inline ServerEntity& GetServerEntity(std::size_t id);
			inline bool IsServerEntityValid(std::size_t id) const;

			void ControlEntity(std::size_t entityId);
			void PrintMessage(const std::string& message);

			void OnArenaState(ServerConnection* server, const Packets::ArenaState& arenaState);
			void OnChatMessage(ServerConnection* server, const Packets::ChatMessage& chatMessage);

			void OnControlEntity(ServerConnection* server, const Packets::ControlEntity& controlPacket);

			void OnCreateEntity(ServerConnection* server, const Packets::CreateEntity& createPacket);
			void OnDeleteEntity(ServerConnection* server, const Packets::DeleteEntity& deletePacket);
			void OnKeyPressed(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::KeyEvent& event);

			void UpdateInput(float elapsedTime);

			static void ApplyInput(Nz::Node& node, Nz::UInt64 lastInputTime, const ClientInput& input);

			NazaraSlot(ServerConnection, OnArenaState, m_onArenaStateSlot);
			NazaraSlot(ServerConnection, OnChatMessage, m_onChatMessageSlot);
			NazaraSlot(ServerConnection, OnControlEntity, m_onControlEntitySlot);
			NazaraSlot(ServerConnection, OnCreateEntity, m_onCreateEntitySlot);
			NazaraSlot(ServerConnection, OnDeleteEntity, m_onDeleteEntitySlot);
			NazaraSlot(Nz::EventHandler, OnKeyPressed, m_onKeyPressedSlot);
			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			static constexpr std::size_t JitterBufferSize = 3;

			StateData& m_stateData;
			Ndk::EntityHandle m_cursorEntity;
			Ndk::TextAreaWidget* m_chatBox;
			Ndk::TextAreaWidget* m_chatEnteringBox;
			Nz::Node m_cameraNode;
			Nz::Node m_spaceshipMovementNode;
			Nz::SpriteRef m_cursorOrientationSprite;
			Nz::Vector2i m_rotationCursorOrigin;
			Nz::Vector2i m_rotationCursorPosition;
			Nz::Vector2f m_rotationDirection;
			Nz::Vector3f m_cameraRotation;
			Nz::Vector3f m_spaceshipRotation;
			Nz::Vector3f m_spaceshipSpeed;
			Nz::UdpSocket m_debugStateSocket;
			Nz::UInt16 m_lastStateId;
			Nz::UInt64 m_lastInputTime;
			std::size_t m_controlledEntity;
			std::array<ServerSnapshot, JitterBufferSize> m_snapshots;
			std::optional<ServerMatchEntities> m_matchEntities;
			std::vector<ClientInput> m_predictedInputs;
			std::vector<ServerEntity> m_serverEntities;
			std::vector<Nz::String> m_chatLines;
			bool m_resetSnapshots;
			bool m_isCurrentlyRotating;
			bool m_syncEnabled;
			float m_inputAccumulator;
			float m_interpolationFactor;
			float m_snapshotUpdateAccumulator;
	};
}

#include <Client/States/GameState.inl>

#endif // EREWHON_CLIENT_STATES_GAMESTATE_HPP
