// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_FLEETLISTSTATE_HPP
#define EREWHON_CLIENT_STATES_FLEETLISTSTATE_HPP

#include <Client/States/AbstractState.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>
#include <future>

namespace ewn
{
	class FleetListState final : public AbstractState
	{
		public:
			inline FleetListState(StateData& stateData, std::shared_ptr<Ndk::State> previousState);
			~FleetListState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;

			void LayoutWidgets() override;
			void QueryFleets();

			void OnBackPressed();
			void OnFleetList(ServerConnection* server, const Packets::FleetList& listPacket);

			void UpdateStatus(const Nz::String& status, const Nz::Color& color = Nz::Color::White);

			Ndk::ButtonWidget* m_backButton;
			Ndk::ButtonWidget* m_createButton;
			Ndk::LabelWidget* m_statusLabel;
			Ndk::LabelWidget* m_titleLabel;
			std::shared_ptr<Ndk::State> m_previousState;
			std::vector<Ndk::BaseWidget*> m_fleetButtons;
	};
}

#include <Client/States/Game/FleetListState.inl>

#endif // EREWHON_CLIENT_STATES_FLEETLISTSTATE_HPP
