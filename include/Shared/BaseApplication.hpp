// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_BASE_APPLICATION_HPP
#define EREWHON_SHARED_BASE_APPLICATION_HPP

#include <NDK/Application.hpp>
#include <Shared/NetworkReactor.hpp>
#include <memory>
#include <vector>

namespace ewn
{
	class BaseApplication : public Ndk::Application
	{
		public:
			BaseApplication() = default;
			virtual ~BaseApplication();

			inline std::size_t GetPeerPerReactor() const;
			inline std::size_t GetReactorCount() const;

			virtual bool Run() = 0;

			void SetupNetwork(std::size_t clientPerReactor, const Nz::IpAddress& ipAddress);

		protected:
			inline const std::unique_ptr<NetworkReactor>& GetReactor(std::size_t reactorId);

			virtual void HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data) = 0;
			virtual void HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data) = 0;
			virtual void HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet) = 0;

		private:
			std::size_t m_peerPerReactor;
			std::vector<std::unique_ptr<NetworkReactor>> m_reactors;
	};
}

#include <Shared/BaseApplication.inl>

#endif // EREWHON_SHARED_BASE_APPLICATION_HPP