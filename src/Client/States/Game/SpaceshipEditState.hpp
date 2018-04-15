// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_SPACESHIPEDITSTATE_HPP
#define EREWHON_CLIENT_STATES_SPACESHIPEDITSTATE_HPP

#include <Client/States/AbstractState.hpp>
#include <Nazara/Graphics/Model.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>
#include <future>

namespace ewn
{
	class SpaceshipEditState final : public AbstractState
	{
		public:
			inline SpaceshipEditState(StateData& stateData, std::shared_ptr<Ndk::State> previousState, std::string spaceshipName);
			~SpaceshipEditState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets();

			void OnBackPressed();
			void OnSpaceshipInfo(ServerConnection* server, const Packets::SpaceshipInfo& listPacket);
			void OnUpdateSpaceshipFailure(ServerConnection* server, const Packets::UpdateSpaceshipFailure& updatePacket);
			void OnUpdateSpaceshipSuccess(ServerConnection* server, const Packets::UpdateSpaceshipSuccess& updatePacket);
			void OnUpdatePressed();

			void QuerySpaceshipInfo();
			void UpdateStatus(const Nz::String& status, const Nz::Color& color = Nz::Color::White);

			NazaraSlot(ServerConnection, OnSpaceshipInfo, m_onSpaceshipInfoSlot);
			NazaraSlot(ServerConnection, OnUpdateSpaceshipFailure, m_onUpdateSpaceshipFailureSlot);
			NazaraSlot(ServerConnection, OnUpdateSpaceshipSuccess, m_onUpdateSpaceshipSuccessSlot);
			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			Ndk::ButtonWidget* m_backButton;
			Ndk::ButtonWidget* m_updateButton;
			Ndk::LabelWidget* m_statusLabel;
			Ndk::LabelWidget* m_titleLabel;
			Ndk::LabelWidget* m_nameLabel;
			Ndk::TextAreaWidget* m_nameTextArea;
			Nz::ModelRef m_spaceshipModel;
			Ndk::EntityOwner m_light;
			Ndk::EntityOwner m_spaceship;
			std::shared_ptr<Ndk::State> m_previousState;
			std::shared_ptr<Ndk::State> m_nextState;
			std::string m_spaceshipName;
	};
}

#include <Client/States/Game/SpaceshipEditState.inl>

#endif // EREWHON_CLIENT_STATES_SPACESHIPEDITSTATE_HPP
