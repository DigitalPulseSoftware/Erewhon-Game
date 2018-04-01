// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_BASE_APPLICATION_HPP
#define EREWHON_SHARED_BASE_APPLICATION_HPP

#include <NDK/Application.hpp>
#include <Shared/ConfigFile.hpp>
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

			inline ConfigFile& GetConfig();
			inline const ConfigFile& GetConfig() const;
			inline std::size_t GetPeerPerReactor() const;
			inline std::size_t GetReactorCount() const;

			inline bool LoadConfig(const std::string& configFile);

			virtual bool Run() = 0;

			bool SetupNetwork(std::size_t clientPerReactor, const Nz::IpAddress& ipAddress);

			static inline Nz::UInt64 GetAppTime();

		protected:
			inline const std::unique_ptr<NetworkReactor>& GetReactor(std::size_t reactorId);

			virtual void HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data) = 0;
			virtual void HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data) = 0;
			virtual void HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet) = 0;
			virtual void OnConfigLoaded(const ConfigFile& config);

			ConfigFile m_config;

		private:
			std::size_t m_peerPerReactor;
			std::vector<std::unique_ptr<NetworkReactor>> m_reactors;

			static Nz::Clock s_appClock;
	};
}

#include <Shared/BaseApplication.inl>

#endif // EREWHON_SHARED_BASE_APPLICATION_HPP
