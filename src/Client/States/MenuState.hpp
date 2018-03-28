// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_MENUSTATE_HPP
#define EREWHON_CLIENT_STATES_MENUSTATE_HPP

#include <Client/States/AbstractState.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>

namespace ewn
{
	class MenuState final : public AbstractState
	{
		public:
			using AbstractState::AbstractState;
			~MenuState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets();

			void OnDisconnectionPressed();
			void OnKeyPressed(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::KeyEvent& event);
			void OnOptionsPressed();

			NazaraSlot(Nz::EventHandler, OnKeyPressed, m_onKeyPressedSlot);
			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			Ndk::ButtonWidget* m_optionsButton;
			Ndk::ButtonWidget* m_disconnectButton;
			Ndk::CheckboxWidget* m_fullscreenCheckbox;
			Ndk::CheckboxWidget* m_vsyncCheckbox;
			bool m_isDisconnecting;
			bool m_isLeavingMenu;
			bool m_isUsingOption;
	};
}

#include <Client/States/MenuState.inl>

#endif // EREWHON_CLIENT_STATES_MENUSTATE_HPP
