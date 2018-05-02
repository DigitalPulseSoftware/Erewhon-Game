// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_ARENASTATE_HPP
#define EREWHON_CLIENT_STATES_ARENASTATE_HPP

#include <Client/MatchChatbox.hpp>
#include <Client/ServerMatchEntities.hpp>
#include <Client/SpaceshipController.hpp>
#include <Client/States/AbstractState.hpp>
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
	class ArenaState final : public AbstractState
	{
		public:
			inline ArenaState(StateData& stateData, Nz::UInt8 arenaIndex);
			~ArenaState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void ControlEntity(std::size_t entityId);

			void OnControlEntity(ServerConnection* server, const Packets::ControlEntity& controlPacket);
			void OnEntityCreated(ServerMatchEntities* entities, ServerMatchEntities::ServerEntity& entityData);
			void OnEntityDelete(ServerMatchEntities* entities, ServerMatchEntities::ServerEntity& entityData);
			void OnKeyPressed(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::KeyEvent& event);

			NazaraSlot(ServerConnection,    OnControlEntity, m_onControlEntitySlot);
			NazaraSlot(ServerMatchEntities, OnEntityCreated, m_onEntityCreatedSlot);
			NazaraSlot(ServerMatchEntities, OnEntityDelete, m_onEntityDeletionSlot);
			NazaraSlot(Nz::EventHandler,    OnKeyPressed, m_onKeyPressedSlot);

			std::optional<MatchChatbox> m_chatbox;
			std::optional<ServerMatchEntities> m_matchEntities;
			std::optional<SpaceshipController> m_spaceshipController;
			std::size_t m_controlledEntity;
			Nz::UInt8 m_arenaIndex;
			bool m_isDisconnected;
			bool m_isEnteringMenu;
	};
}

#include <Client/States/Game/ArenaState.inl>

#endif // EREWHON_CLIENT_STATES_ARENASTATE_HPP
