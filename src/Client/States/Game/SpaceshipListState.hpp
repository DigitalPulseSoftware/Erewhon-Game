// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_SPACESHIPLISTSTATE_HPP
#define EREWHON_CLIENT_STATES_SPACESHIPLISTSTATE_HPP

#include <Client/States/AbstractState.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>
#include <future>

namespace ewn
{
	class SpaceshipListState final : public AbstractState
	{
		public:
			inline SpaceshipListState(StateData& stateData, std::shared_ptr<Ndk::State> previousState);
			~SpaceshipListState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets();
			void QuerySpaceships();

			void OnBackPressed();
			void OnSpaceshipList(ServerConnection* server, const Packets::SpaceshipList& listPacket);

			void UpdateStatus(const Nz::String& status, const Nz::Color& color = Nz::Color::White);

			NazaraSlot(ServerConnection, OnSpaceshipList, m_onSpaceshipListSlot);
			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			Ndk::ButtonWidget* m_backButton;
			Ndk::LabelWidget* m_statusLabel;
			Ndk::LabelWidget* m_titleLabel;
			std::shared_ptr<Ndk::State> m_previousState;
			std::shared_ptr<Ndk::State> m_nextState;
			std::vector<Ndk::BaseWidget*> m_spaceshipButtons;
	};
}

#include <Client/States/Game/SpaceshipListState.inl>

#endif // EREWHON_CLIENT_STATES_SPACESHIPLISTSTATE_HPP
