// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/ConnectedState.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/ConnectionLostState.hpp>

namespace ewn
{
	void ConnectedState::Enter(Ndk::StateMachine& fsm)
	{
	}

	void ConnectedState::Leave(Ndk::StateMachine& fsm)
	{
	}

	bool ConnectedState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!m_stateData.server->IsConnected())
		{
			fsm.ResetState(std::make_shared<BackgroundState>(m_stateData));
			fsm.PushState(std::make_shared<ConnectionLostState>(m_stateData));
			return false;
		}

		return true;
	}
}
