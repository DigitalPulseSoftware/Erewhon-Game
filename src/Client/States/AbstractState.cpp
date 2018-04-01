// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/AbstractState.hpp>

namespace ewn
{
	void AbstractState::Leave(Ndk::StateMachine& /*fsm*/)
	{
		for (Ndk::BaseWidget* widget : m_widgets)
			widget->Destroy();
	}
}
