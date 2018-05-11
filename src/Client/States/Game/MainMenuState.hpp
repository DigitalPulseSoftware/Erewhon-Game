// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_MAINMENUSTATE_HPP
#define EREWHON_CLIENT_STATES_MAINMENUSTATE_HPP

#include <Client/States/AbstractState.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>
#include <future>

namespace ewn
{
	class MainMenuState final : public AbstractState
	{
		public:
			inline MainMenuState(StateData& stateData, std::string playerName);
			~MainMenuState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;

			void LayoutWidgets() override;

			void OnArenaButtonPressed(std::size_t arenaId);
			void OnArenaList(ServerConnection* server, const Packets::ArenaList& arenaList);
			void OnDisconnectPressed();
			void OnRefreshPressed();
			void OnSpaceshipFactoryPressed();

			Ndk::ButtonWidget* m_disconnectButton;
			Ndk::ButtonWidget* m_refreshButton;
			Ndk::ButtonWidget* m_spaceshipButton;
			Ndk::LabelWidget* m_welcomeTextLabel;
			std::string m_playerName;
			std::vector<Ndk::ButtonWidget*> m_arenaButtons;
	};
}

#include <Client/States/Game/MainMenuState.inl>

#endif // EREWHON_CLIENT_STATES_MAINMENUSTATE_HPP
