// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_GAMESTATE_HPP
#define EREWHON_CLIENT_STATES_GAMESTATE_HPP

#include <Client/ServerMatchEntities.hpp>
#include <Client/SpaceshipController.hpp>
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
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void ControlEntity(std::size_t entityId);

			void PrintMessage(const std::string& message);

			void OnChatMessage(ServerConnection* server, const Packets::ChatMessage& chatMessage);
			void OnControlEntity(ServerConnection* server, const Packets::ControlEntity& controlPacket);
			void OnEntityCreated(ServerMatchEntities* entities, ServerMatchEntities::ServerEntity& entityData);
			void OnEntityDelete(ServerMatchEntities* entities, ServerMatchEntities::ServerEntity& entityData);
			void OnKeyPressed(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::KeyEvent& event);
			void OnRenderTargetSizeChange(const Nz::RenderTarget* renderTarget);

			NazaraSlot(ServerConnection,    OnChatMessage, m_onChatMessageSlot);
			NazaraSlot(ServerConnection,    OnControlEntity, m_onControlEntitySlot);
			NazaraSlot(ServerConnection,    OnIntegrityUpdate, m_onIntegrityUpdateSlot);
			NazaraSlot(ServerMatchEntities, OnEntityCreated, m_onEntityCreatedSlot);
			NazaraSlot(ServerMatchEntities, OnEntityDelete, m_onEntityDeletionSlot);
			NazaraSlot(Nz::EventHandler,    OnKeyPressed, m_onKeyPressedSlot);
			NazaraSlot(Nz::RenderTarget,    OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			StateData& m_stateData;
			Ndk::TextAreaWidget* m_chatBox;
			Ndk::TextAreaWidget* m_chatEnteringBox;
			std::optional<ServerMatchEntities> m_matchEntities;
			std::optional<SpaceshipController> m_spaceshipController;
			std::size_t m_controlledEntity;
			std::vector<Nz::String> m_chatLines;
			bool m_isDisconnected;
	};
}

#include <Client/States/GameState.inl>

#endif // EREWHON_CLIENT_STATES_GAMESTATE_HPP
