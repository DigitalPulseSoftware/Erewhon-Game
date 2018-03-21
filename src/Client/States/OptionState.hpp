// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_OPTIONSTATE_HPP
#define EREWHON_CLIENT_STATES_OPTIONSTATE_HPP

#include <Client/States/StateData.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>

namespace ewn
{
	class OptionState final : public Ndk::State
	{
		public:
			inline OptionState(StateData& stateData);
			~OptionState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets();

			void OnApplyPressed();
			void OnBackPressed();

			void ApplyOptions();
			void LoadOptions();
			void SaveOptions();

			void UpdateStatus(const Nz::String& status, const Nz::Color& color);

			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			StateData& m_stateData;
			Ndk::ButtonWidget* m_applyButton;
			Ndk::ButtonWidget* m_backButton;
			Ndk::CheckboxWidget* m_fullscreenCheckbox;
			Ndk::CheckboxWidget* m_vsyncCheckbox;
			Ndk::LabelWidget* m_statusLabel;
			bool m_isReturningBack;
	};
}

#include <Client/States/OptionState.inl>

#endif // EREWHON_CLIENT_STATES_OPTIONSTATE_HPP
