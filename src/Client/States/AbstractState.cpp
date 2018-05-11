// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/AbstractState.hpp>

namespace ewn
{
	void AbstractState::Enter(Ndk::StateMachine& fsm)
	{
		m_onTargetChangeSizeSlot.Connect(m_stateData.window->OnRenderTargetSizeChange, [this](const Nz::RenderTarget*) { LayoutWidgets(); });
	}

	void AbstractState::Leave(Ndk::StateMachine& /*fsm*/)
	{
		for (const auto& cleanupFunc : m_cleanupFunctions)
			cleanupFunc();

		m_cleanupFunctions.clear();

		m_onTargetChangeSizeSlot.Disconnect();

		for (Ndk::BaseWidget* widget : m_widgets)
			widget->Destroy();

		m_widgets.clear();
	}

	bool AbstractState::Update(Ndk::StateMachine& /*fsm*/, float /*elapsedTime*/)
	{
		return true;
	}

	void AbstractState::LayoutWidgets()
	{
	}
}
