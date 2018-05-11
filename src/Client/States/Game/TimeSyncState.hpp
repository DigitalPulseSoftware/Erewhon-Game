// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_TIMESYNCSTATE_HPP
#define EREWHON_CLIENT_STATES_TIMESYNCSTATE_HPP

#include <Client/ClientApplication.hpp>
#include <Client/States/AbstractState.hpp>
#include <Nazara/Core/Clock.hpp>
#include <Nazara/Graphics/TextSprite.hpp>
#include <Nazara/Renderer/RenderTarget.hpp>
#include <NDK/EntityOwner.hpp>
#include <NDK/State.hpp>
#include <NDK/World.hpp>
#include <vector>

namespace ewn
{
	class TimeSyncState final : public AbstractState
	{
		public:
			inline TimeSyncState(StateData& stateData, Nz::UInt8 arenaIndex);
			~TimeSyncState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets() override;

			void OnTimeSyncResponse(ServerConnection* server, const Packets::TimeSyncResponse& response);
			void UpdateStatus(const Nz::String& status, const Nz::Color& color = Nz::Color::White);

			Ndk::EntityOwner m_statusText;
			Nz::TextSpriteRef m_statusSprite;
			Nz::UInt8 m_expectedRequestId;
			Nz::UInt64 m_pingAccumulator;
			Nz::UInt64 m_requestTime;
			Nz::UInt8 m_arenaIndex;
			std::vector<Nz::UInt64> m_results;
			bool m_finished;
			bool m_isClientYounger;
			float m_accumulator;
			float m_nextStepTime;
	};
}

#include <Client/States/Game/TimeSyncState.inl>

#endif // EREWHON_CLIENT_STATES_TIMESYNCSTATE_HPP
