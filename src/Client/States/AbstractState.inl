// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/AbstractState.hpp>

namespace ewn
{
	inline AbstractState::AbstractState(StateData& stateData) :
	m_stateData(stateData)
	{
	}

	inline StateData& AbstractState::GetStateData()
	{
		return m_stateData;
	}

	inline const StateData& AbstractState::GetStateData() const
	{
		return m_stateData;
	}

	template<typename T, typename... Args>
	T* AbstractState::CreateWidget(Args&&... args)
	{
		T* widget = m_stateData.canvas->Add<T>(std::forward<Args>(args)...);
		m_widgets.push_back(widget);

		return widget;
	}
}
