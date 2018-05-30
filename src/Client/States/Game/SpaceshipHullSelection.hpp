// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_SPACESHIPHULLSELECTION_HPP
#define EREWHON_CLIENT_STATES_SPACESHIPHULLSELECTION_HPP

#include <Client/States/AbstractState.hpp>
#include <Shared/Enums.hpp>
#include <Nazara/Graphics/Model.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>
#include <array>

namespace ewn
{
	class SpaceshipHullSelection final : public AbstractState
	{
		public:
			inline SpaceshipHullSelection(StateData& stateData, std::shared_ptr<Ndk::State> previousState);
			~SpaceshipHullSelection() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets() override;

			void QueryHullList();

			// GUI Event
			void OnBackPressed();
			void OnHullSwitch(std::size_t hullId);
			void OnSelectPressed();

			// Network event
			void OnHullList(ServerConnection* server, const Packets::HullList& moduleList);

			struct HullInfo
			{
				Ndk::EntityOwner hullEntity;
				Ndk::ButtonWidget* button;
				Nz::ModelRef hullModel;
				Nz::UInt32 hullId;
				std::string hullName;
				std::string hullDescription;
				std::string hullPath;
				std::string hullSlotsDescription;
			};

			Ndk::ButtonWidget* m_backButton;
			Ndk::ButtonWidget* m_selectButton;
			Ndk::LabelWidget* m_descriptionLabel;
			Ndk::LabelWidget* m_nameLabel;
			Ndk::LabelWidget* m_slotLabel;
			Ndk::EntityOwner m_light;
			Ndk::EntityOwner m_selectedHull;
			std::shared_ptr<Ndk::State> m_previousState;
			std::size_t m_selectedHullIndex;
			std::vector<HullInfo> m_hullInfo;
	};
}

#include <Client/States/Game/SpaceshipHullSelection.inl>

#endif // EREWHON_CLIENT_STATES_SPACESHIPHULLSELECTION_HPP
