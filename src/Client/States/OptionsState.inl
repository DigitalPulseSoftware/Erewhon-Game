// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/OptionsState.hpp>

namespace ewn
{
	inline OptionsState::OptionsState(StateData& stateData, std::shared_ptr<Ndk::State> previousState) :
	AbstractState(stateData),
	m_previousState(std::move(previousState))
	{
	}
}
