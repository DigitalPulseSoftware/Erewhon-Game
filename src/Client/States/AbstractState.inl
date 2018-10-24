// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/AbstractState.hpp>
#include <cassert>

namespace ewn
{
	inline AbstractState::AbstractState(StateData& stateData) :
	m_stateData(stateData)
	{
	}

	template<typename T, typename ...Args>
	void AbstractState::ConnectSignal(T& signal, Args&&... args)
	{
		m_cleanupFunctions.emplace_back([connection = signal.Connect(std::forward<Args>(args)...)]() mutable { connection.Disconnect(); });
	}

	template<typename T, typename... Args>
	T* AbstractState::CreateWidget(Args&&... args)
	{
		T* widget = m_stateData.canvas->Add<T>(std::forward<Args>(args)...);
		m_widgets.push_back(widget);

		return widget;
	}

	inline void AbstractState::DestroyWidget(Ndk::BaseWidget* widget)
	{
		auto it = std::find(m_widgets.begin(), m_widgets.end(), widget);
		assert(it != m_widgets.end());

		m_widgets.erase(it);

		widget->Destroy();
	}

	inline StateData& AbstractState::GetStateData()
	{
		return m_stateData;
	}

	inline const StateData& AbstractState::GetStateData() const
	{
		return m_stateData;
	}
}
