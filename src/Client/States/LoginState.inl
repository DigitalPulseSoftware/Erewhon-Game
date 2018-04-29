// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/LoginState.hpp>

namespace ewn
{
	inline LoginState::LoginState(StateData & stateData, bool shouldAutoLogin) :
	AbstractState(stateData),
	m_shouldAutoLogin(shouldAutoLogin)
	{
	}
}
