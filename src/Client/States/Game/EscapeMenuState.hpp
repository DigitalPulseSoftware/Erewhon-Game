// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_ESCAPEMENUSTATE_HPP
#define EREWHON_CLIENT_STATES_ESCAPEMENUSTATE_HPP

#include <Client/States/AbstractState.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>

namespace ewn
{
	class EscapeMenuState final : public AbstractState
	{
		public:
			using AbstractState::AbstractState;
			~EscapeMenuState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;

			void LayoutWidgets() override;

			void OnKeyPressed(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::KeyEvent& event);

			Ndk::ButtonWidget* m_disconnectButton;
			Ndk::ButtonWidget* m_leaveButton;
			Ndk::ButtonWidget* m_optionsButton;
			Ndk::ButtonWidget* m_quitButton;
			Ndk::CheckboxWidget* m_fullscreenCheckbox;
			Ndk::CheckboxWidget* m_vsyncCheckbox;
	};
}

#include <Client/States/Game/EscapeMenuState.inl>

#endif // EREWHON_CLIENT_STATES_ESCAPEMENUSTATE_HPP
