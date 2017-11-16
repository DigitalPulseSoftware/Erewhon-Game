// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_APPLICATION_HPP
#define EREWHON_CLIENT_APPLICATION_HPP

#include <Nazara/Core/Signal.hpp>
#include <Nazara/Network/ENetHost.hpp>
#include <Shared/BaseApplication.hpp>
#include <Shared/NetworkReactor.hpp>
#include <Client/ClientCommandStore.hpp>
#include <memory>
#include <vector>

namespace ewn
{
	class ClientApplication final : public BaseApplication
	{
		public:
			ClientApplication();
			virtual ~ClientApplication();

			bool Connect(const Nz::String& serverHostname, Nz::UInt32 data = 0);

			inline bool IsConnected() const;

			bool Run() override;

			template<typename T> void SendPacket(const T& packet);

			// TEMPORARY
			void HandleArenaState(std::size_t peerId, const Packets::ArenaState& data);
			void HandleControlSpaceship(std::size_t peerId, const Packets::ControlSpaceship& data);
			void HandleCreateSpaceship(std::size_t peerId, const Packets::CreateSpaceship& data);
			void HandleDeleteSpaceship(std::size_t peerId, const Packets::DeleteSpaceship& data);
			void HandleLoginFailure(std::size_t peerId, const Packets::LoginFailure& data);
			void HandleLoginSuccess(std::size_t peerId, const Packets::LoginSuccess& data);
			NazaraSignal(OnArenaState, const Packets::ArenaState& /*data*/);
			NazaraSignal(OnControlSpaceship, const Packets::ControlSpaceship& /*data*/);
			NazaraSignal(OnCreateSpaceship, const Packets::CreateSpaceship& /*data*/);
			NazaraSignal(OnDeleteSpaceship, const Packets::DeleteSpaceship& /*data*/);
			NazaraSignal(OnLoginFailed, const Packets::LoginFailure& /*data*/);
			NazaraSignal(OnLoginSucceeded, const Packets::LoginSuccess& /*data*/);


			NazaraSignal(OnServerConnected, Nz::UInt32 /*data*/);
			NazaraSignal(OnServerDisconnected, Nz::UInt32 /*data*/);

		private:
			void HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data) override;
			void HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data) override;
			void HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet) override;

			ClientCommandStore m_commandStore;
			std::size_t m_serverPeerId;
			bool m_isServerConnected;
	};
}

#include <Client/ClientApplication.inl>

#endif // EREWHON_CLIENT_APPLICATION_HPP
