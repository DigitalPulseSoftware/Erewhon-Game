// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_OPTIONSSTATE_HPP
#define EREWHON_CLIENT_STATES_OPTIONSSTATE_HPP

#include <Client/States/AbstractState.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>

namespace ewn
{
	class OptionsState final : public AbstractState
	{
		public:
			inline OptionsState(StateData& stateData, std::shared_ptr<Ndk::State> previousState);
			~OptionsState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets() override;

			void OnApplyPressed();
			void OnBackPressed();

			void ApplyOptions();
			void LoadOptions();
			void SaveOptions();

			void UpdateStatus(const Nz::String& status, const Nz::Color& color);

			std::shared_ptr<Ndk::State> m_previousState;
			Ndk::ButtonWidget* m_applyButton;
			Ndk::ButtonWidget* m_backButton;
			Ndk::CheckboxWidget* m_forceIPv4Checkbox;
			Ndk::CheckboxWidget* m_fullscreenCheckbox;
			Ndk::CheckboxWidget* m_vsyncCheckbox;
			Ndk::LabelWidget* m_statusLabel;
			bool m_isReturningBack;
	};
}

#include <Client/States/OptionsState.inl>

#endif // EREWHON_CLIENT_STATES_OPTIONSSTATE_HPP
