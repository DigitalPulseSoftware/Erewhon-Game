// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_ABSTRACTSTATE_HPP
#define EREWHON_CLIENT_STATES_ABSTRACTSTATE_HPP

#include <Client/States/StateData.hpp>
#include <NDK/BaseWidget.hpp>
#include <NDK/State.hpp>
#include <vector>

namespace ewn
{
	class AbstractState : public Ndk::State
	{
		public:
			inline AbstractState(StateData& stateData);
			~AbstractState() = default;

		protected:
			template<typename T, typename... Args> T* CreateWidget(Args&&... args);
			inline void DestroyWidget(Ndk::BaseWidget* widget);

			inline StateData& GetStateData();
			inline const StateData& GetStateData() const;

			void Leave(Ndk::StateMachine& fsm) override;

		private:
			StateData m_stateData;
			std::vector<Ndk::BaseWidget*> m_widgets;
	};
}

#include <Client/States/AbstractState.inl>

#endif // EREWHON_CLIENT_STATES_ABSTRACTSTATE_HPP
