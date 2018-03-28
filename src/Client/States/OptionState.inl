// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/OptionState.hpp>

namespace ewn
{
	inline OptionState::OptionState(StateData& stateData, std::shared_ptr<Ndk::State> previousState) :
	AbstractState(stateData),
	m_previousState(std::move(previousState))
	{
	}
}
