// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_ABSTRACTSTATE_HPP
#define EREWHON_CLIENT_STATES_ABSTRACTSTATE_HPP

#include <Client/States/StateData.hpp>
#include <NDK/BaseWidget.hpp>
#include <NDK/State.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace ewn
{
	class AbstractState : public Ndk::State, public std::enable_shared_from_this<AbstractState>
	{
		public:
			inline AbstractState(StateData& stateData);
			~AbstractState() = default;

		protected:
			template<typename T, typename... Args> void ConnectSignal(T& signal, Args&&... args);
			template<typename T, typename... Args> T* CreateWidget(Args&&... args);
			inline void DestroyWidget(Ndk::BaseWidget* widget);

			inline StateData& GetStateData();
			inline const StateData& GetStateData() const;

			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			virtual void LayoutWidgets();

		private:
			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			StateData& m_stateData;
			std::vector<std::function<void()>> m_cleanupFunctions;
			std::vector<Ndk::BaseWidget*> m_widgets;
	};
}

#include <Client/States/AbstractState.inl>

#endif // EREWHON_CLIENT_STATES_ABSTRACTSTATE_HPP
