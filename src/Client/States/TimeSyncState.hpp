// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_TIMESYNCSTATE_HPP
#define EREWHON_CLIENT_STATES_TIMESYNCSTATE_HPP

#include <Client/ClientApplication.hpp>
#include <Client/States/StateData.hpp>
#include <Nazara/Core/Clock.hpp>
#include <Nazara/Graphics/TextSprite.hpp>
#include <Nazara/Renderer/RenderTarget.hpp>
#include <NDK/EntityOwner.hpp>
#include <NDK/State.hpp>
#include <NDK/World.hpp>
#include <vector>

namespace ewn
{
	class TimeSyncState final : public Ndk::State
	{
		public:
			inline TimeSyncState(StateData& stateData);
			~TimeSyncState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void CenterStatus();
			void OnServerDisconnected(ServerConnection* server, Nz::UInt32 data);
			void OnTimeSyncResponse(ServerConnection* server, const Packets::TimeSyncResponse& response);
			void UpdateStatus(const Nz::String& status, const Nz::Color& color = Nz::Color::White);

			NazaraSlot(ServerConnection, OnTimeSyncResponse, m_onTimeSyncResponseSlot);
			NazaraSlot(ServerConnection, OnDisconnected, m_onServerDisconnectedSlot);
			NazaraSlot(Nz::RenderTarget,  OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			StateData& m_stateData;
			Ndk::EntityOwner m_statusText;
			Nz::TextSpriteRef m_statusSprite;
			Nz::UInt8 m_expectedRequestId;
			Nz::UInt64 m_requestTime;
			std::vector<Nz::UInt64> m_results;
			bool m_connected;
			bool m_finished;
			bool m_isClientYounger;
			float m_accumulator;
			float m_nextStepTime;
	};
}

#include <Client/States/TimeSyncState.inl>

#endif // EREWHON_CLIENT_STATES_TIMESYNCSTATE_HPP
