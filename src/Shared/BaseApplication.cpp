// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/BaseApplication.hpp>
#include <iostream>

namespace ewn
{
	BaseApplication::~BaseApplication() = default;

	bool BaseApplication::Run()
	{
		m_appTime.store(m_appClock.GetMilliseconds(), std::memory_order_relaxed);

		for (const auto& reactorPtr : m_reactors)
		{
			reactorPtr->Poll([&](bool outgoing, std::size_t clientId, Nz::UInt32 data) { HandlePeerConnection(outgoing, clientId, data); },
			                 [&](std::size_t clientId, Nz::UInt32 data) { HandlePeerDisconnection(clientId, data); },
			                 [&](std::size_t clientId, Nz::NetPacket&& packet) { HandlePeerPacket(clientId, std::move(packet)); },
			                 [&](std::size_t clientId, const NetworkReactor::PeerInfo& peerInfo) { HandlePeerInfo(clientId, peerInfo); });
		}

		return Application::Run();
	}

	void BaseApplication::HandlePeerInfo(std::size_t peerId, const NetworkReactor::PeerInfo& peerInfo)
	{
	}

	void BaseApplication::OnConfigLoaded(const ConfigFile& /*config*/)
	{
	}
}
