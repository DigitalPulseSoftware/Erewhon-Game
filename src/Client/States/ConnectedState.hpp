// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_CONNECTEDSTATE_HPP
#define EREWHON_CLIENT_STATES_CONNECTEDSTATE_HPP

#include <Client/States/StateData.hpp>
#include <NDK/State.hpp>

namespace ewn
{
	class ConnectedState : public Ndk::State
	{
		public:
			inline ConnectedState(StateData& stateData);
			~ConnectedState() = default;

		protected:
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			StateData& m_stateData;
	};
}

#include <Client/States/ConnectedState.inl>

#endif // EREWHON_CLIENT_STATES_CONNECTEDSTATE_HPP
