// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_GAMESTATE_HPP
#define EREWHON_CLIENT_STATES_GAMESTATE_HPP

#include <Client/ServerMatchEntities.hpp>
#include <Client/States/StateData.hpp>
#include <Nazara/Audio/Sound.hpp>
#include <Nazara/Core/Clock.hpp>
#include <Nazara/Graphics/Sprite.hpp>
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

			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void ControlEntity(std::size_t entityId);

			void PrintMessage(const std::string& message);

			void OnChatMessage(ServerConnection* server, const Packets::ChatMessage& chatMessage);
			void OnControlEntity(ServerConnection* server, const Packets::ControlEntity& controlPacket);
			void OnEntityCreated(ServerMatchEntities* entities, ServerMatchEntities::ServerEntity& entityData);
			void OnEntityDelete(ServerMatchEntities* entities, ServerMatchEntities::ServerEntity& entityData);
			void OnIntegrityUpdate(ServerConnection* server, const Packets::IntegrityUpdate& integrityUpdate);
			void OnKeyPressed(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::KeyEvent& event);
			void OnRenderTargetSizeChange(const Nz::RenderTarget* renderTarget);

			void UpdateInput(float elapsedTime);

			static void ApplyInput(Nz::Node& node, Nz::UInt64 lastInputTime, const ClientInput& input);

			NazaraSlot(ServerConnection,    OnChatMessage, m_onChatMessageSlot);
			NazaraSlot(ServerConnection,    OnControlEntity, m_onControlEntitySlot);
			NazaraSlot(ServerConnection,    OnIntegrityUpdate, m_onIntegrityUpdateSlot);
			NazaraSlot(ServerMatchEntities, OnEntityCreated, m_onEntityCreatedSlot);
			NazaraSlot(ServerMatchEntities, OnEntityDelete, m_onEntityDeletionSlot);
			NazaraSlot(Nz::EventHandler,    OnKeyPressed, m_onKeyPressedSlot);
			NazaraSlot(Nz::RenderTarget,    OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			StateData& m_stateData;
			Ndk::EntityHandle m_crosshairEntity;
			Ndk::EntityHandle m_cursorEntity;
			Ndk::EntityHandle m_healthBarEntity;
			Ndk::TextAreaWidget* m_chatBox;
			Ndk::TextAreaWidget* m_chatEnteringBox;
			Nz::Node m_cameraNode;
			Nz::Node m_spaceshipMovementNode;
			Nz::Sound m_shootSound;
			Nz::SpriteRef m_cursorOrientationSprite;
			Nz::SpriteRef m_healthBarSprite;
			Nz::Vector2i m_rotationCursorOrigin;
			Nz::Vector2i m_rotationCursorPosition;
			Nz::Vector2f m_rotationDirection;
			Nz::Vector3f m_cameraRotation;
			Nz::Vector3f m_spaceshipRotation;
			Nz::Vector3f m_spaceshipSpeed;
			Nz::UInt64 m_lastInputTime;
			Nz::UInt64 m_lastShootTime;
			std::size_t m_controlledEntity;
			std::optional<ServerMatchEntities> m_matchEntities;
			std::vector<ClientInput> m_predictedInputs;
			std::vector<Nz::String> m_chatLines;
			bool m_isCurrentlyRotating;
			bool m_isDisconnected;
			float m_inputAccumulator;
	};
}

#include <Client/States/GameState.inl>

#endif // EREWHON_CLIENT_STATES_GAMESTATE_HPP
