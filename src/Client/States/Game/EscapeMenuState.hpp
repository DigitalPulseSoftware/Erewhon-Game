// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
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
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets();

			void OnKeyPressed(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::KeyEvent& event);

			NazaraSlot(Nz::EventHandler, OnKeyPressed, m_onKeyPressedSlot);
			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			Ndk::ButtonWidget* m_disconnectButton;
			Ndk::ButtonWidget* m_optionsButton;
			Ndk::ButtonWidget* m_quitButton;
			Ndk::CheckboxWidget* m_fullscreenCheckbox;
			Ndk::CheckboxWidget* m_vsyncCheckbox;
	};
}

#include <Client/States/Game/EscapeMenuState.inl>

#endif // EREWHON_CLIENT_STATES_ESCAPEMENUSTATE_HPP
