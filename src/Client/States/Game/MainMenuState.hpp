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
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets();

			void OnPlayPressed();
			void OnSpaceshipFactoryPressed();

			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			Ndk::ButtonWidget* m_playButton;
			Ndk::ButtonWidget* m_spaceshipButton;
			Ndk::LabelWidget* m_welcomeTextLabel;
			std::shared_ptr<Ndk::State> m_nextState;
			std::string m_playerName;
	};
}

#include <Client/States/Game/MainMenuState.inl>

#endif // EREWHON_CLIENT_STATES_MAINMENUSTATE_HPP
