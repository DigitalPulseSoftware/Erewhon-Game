// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/NetworkReactor.hpp>
#include <Shared/Config.hpp>
#include <Shared/Utils.hpp>
#include <cassert>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <stdexcept>

namespace ewn
{
	NetworkReactor::NetworkReactor(std::size_t firstId, Nz::NetProtocol protocol, Nz::UInt16 port, std::size_t maxClient) :
	m_firstId(firstId),
	m_protocol(protocol)
	{
		if (port > 0)
		{
			ENetAddress address;
			address.host = ENET_HOST_ANY;
			address.port = port;

			m_host = enet_host_create(&address, maxClient, NetworkChannelCount, 0, 0);
		}
		else
		{
			m_host = enet_host_create(nullptr, maxClient, NetworkChannelCount, 0, 0);
		}

		if (!m_host)
			throw std::runtime_error("Failed to start reactor");

		m_clients.resize(maxClient, nullptr);

		m_running.store(true, std::memory_order_release);
		m_thread = Nz::Thread(&NetworkReactor::WorkerThread, this);
		m_thread.SetName("NetworkReactor");
	}

	NetworkReactor::~NetworkReactor()
	{
		m_running.store(false, std::memory_order_relaxed);
		m_thread.Join();
	}

	std::size_t NetworkReactor::ConnectTo(Nz::IpAddress address, Nz::UInt32 data)
	{
		// We will need a few synchronization primitives to block the calling thread until the reactor has treated our request
		std::condition_variable signal;
		std::mutex signalMutex;

		ConnectionRequest request;
		request.data = data;
		request.remoteAddress = std::move(address);

		std::size_t newClientId = InvalidPeerId;
		bool hasReturned = false;
		request.callback = [&](std::size_t peerId)
		{
			// This callback is called from within the reactor
			newClientId = m_firstId + peerId;
			hasReturned = true;

			std::unique_lock<std::mutex> lock(signalMutex);
			signal.notify_all();
		};

		// Lock before enqueuing the request, to prevent notify being called before we actually wait on the signal
		std::unique_lock<std::mutex> lock(signalMutex);
		m_connectionRequests.enqueue(request);

		// As InvalidClientId is a possible return from the callback, we need another variable to prevent spurious wakeup
		signal.wait(lock, [&]() { return hasReturned; });

		return newClientId;
	}

	void NetworkReactor::DisconnectPeer(std::size_t peerId, Nz::UInt32 data, DisconnectionType type)
	{
		assert(peerId >= m_firstId);

		OutgoingEvent::DisconnectEvent disconnectEvent;
		disconnectEvent.data = data;
		disconnectEvent.type = type;

		OutgoingEvent outgoingData;
		outgoingData.peerId = peerId - m_firstId;
		outgoingData.data = std::move(disconnectEvent);

		m_outgoingQueue.enqueue(std::move(outgoingData));
	}

	void NetworkReactor::QueryInfo(std::size_t peerId)
	{
		assert(peerId >= m_firstId);

		OutgoingEvent outgoingRequest;
		outgoingRequest.peerId = peerId - m_firstId;
		outgoingRequest.data.emplace<OutgoingEvent::QueryPeerInfo>();

		m_outgoingQueue.enqueue(std::move(outgoingRequest));
	}

	void NetworkReactor::SendData(std::size_t peerId, Nz::UInt8 channelId, Nz::ENetPacketFlags flags, Nz::NetPacket&& packet)
	{
		assert(peerId >= m_firstId);

		OutgoingEvent::PacketEvent packetEvent;
		packetEvent.channelId = channelId;
		packetEvent.packet = std::move(packet);
		packetEvent.flags = flags;

		OutgoingEvent outgoingData;
		outgoingData.peerId = peerId - m_firstId;
		outgoingData.data = std::move(packetEvent);

		m_outgoingQueue.enqueue(std::move(outgoingData));
	}

	void NetworkReactor::WorkerThread()
	{
		moodycamel::ConsumerToken connectionToken(m_connectionRequests);
		moodycamel::ConsumerToken outgoingToken(m_outgoingQueue);
		moodycamel::ProducerToken incomingToken(m_incomingQueue);

		while (m_running.load(std::memory_order_acquire))
		{
			ReceivePackets(incomingToken);
			SendPackets(incomingToken, outgoingToken);

			// Handle connection requests last to treat disconnection request before connection requests
			HandleConnectionRequests(connectionToken);
		}
	}

	void NetworkReactor::HandleConnectionRequests(const moodycamel::ConsumerToken& token)
{
		ConnectionRequest request;
		while (m_connectionRequests.try_dequeue(request))
		{
			Nz::IpAddress withoutPort = request.remoteAddress;
			withoutPort.SetPort(0);

			ENetAddress peerAddress;
			enet_address_set_host(&peerAddress, withoutPort.ToString().GetConstBuffer());
			peerAddress.port = request.remoteAddress.GetPort();

			if (ENetPeer* peer = enet_host_connect(m_host, &peerAddress, NetworkChannelCount, request.data))
			{
				Nz::UInt16 peerId = peer->incomingPeerID;
				m_clients[peerId] = peer;

				request.callback(peerId);
			}
			else
				request.callback(InvalidPeerId);
		}
	}

	void NetworkReactor::ReceivePackets(const moodycamel::ProducerToken& producterToken)
	{
		ENetEvent event;
		if (enet_host_service(m_host, &event, 0) > 0)
		{
			do
			{
				Nz::UInt16 peerId = event.peer->incomingPeerID;

				switch (event.type)
				{
					case ENET_EVENT_TYPE_DISCONNECT:
					{
						m_clients[peerId] = nullptr;

						IncomingEvent::DisconnectEvent disconnectEvent;
						disconnectEvent.data = event.data;

						IncomingEvent newEvent;
						newEvent.peerId = m_firstId + peerId;
						newEvent.data.emplace<IncomingEvent::DisconnectEvent>(std::move(disconnectEvent));

						m_incomingQueue.enqueue(producterToken, std::move(newEvent));
						break;
					}

					case ENET_EVENT_TYPE_CONNECT:
					{
						m_clients[peerId] = event.peer;

						IncomingEvent::ConnectEvent connectEvent;
						connectEvent.data = event.data;
						//connectEvent.outgoingConnection = (event.type == Nz::ENetEventType::OutgoingConnect);
						connectEvent.outgoingConnection = false;

						IncomingEvent newEvent;
						newEvent.peerId = m_firstId + peerId;
						newEvent.data.emplace<IncomingEvent::ConnectEvent>(std::move(connectEvent));

						m_incomingQueue.enqueue(producterToken, std::move(newEvent));
						break;
					}

					case ENET_EVENT_TYPE_RECEIVE:
					{
						IncomingEvent::PacketEvent packetEvent;
						packetEvent.packet.Reset(0, event.packet->data, event.packet->dataLength);

						IncomingEvent newEvent;
						newEvent.peerId = m_firstId + peerId;
						newEvent.data.emplace<IncomingEvent::PacketEvent>(std::move(packetEvent));

						m_incomingQueue.enqueue(producterToken, std::move(newEvent));
						break;
					}

					default:
						break;
				}
			}
			while (enet_host_check_events(m_host, &event));
		}
	}

	void NetworkReactor::SendPackets(const moodycamel::ProducerToken& producterToken, const moodycamel::ConsumerToken& token)
	{
		OutgoingEvent outEvent;
		while (m_outgoingQueue.try_dequeue(outEvent))
		{
			std::visit([&](auto&& arg) {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, OutgoingEvent::DisconnectEvent>)
				{
					if (ENetPeer* peer = m_clients[outEvent.peerId])
					{
						switch (arg.type)
						{
							case DisconnectionType::Kick:
							{
								enet_peer_disconnect_now(peer, arg.data);

								// DisconnectNow does not generate Disconnect event
								m_clients[outEvent.peerId] = nullptr;

								IncomingEvent newEvent;
								newEvent.peerId = m_firstId + outEvent.peerId;

								auto& disconnectEvent = newEvent.data.emplace<IncomingEvent::DisconnectEvent>();
								disconnectEvent.data = 0;

								m_incomingQueue.enqueue(producterToken, std::move(newEvent));
								break;
							}

							case DisconnectionType::Later:
								enet_peer_disconnect_later(peer, arg.data);
								break;

							case DisconnectionType::Normal:
								enet_peer_disconnect(peer, arg.data);
								break;

							default:
								assert(!"Unknown disconnection type");
								break;
						}
					}
				}
				else if constexpr (std::is_same_v<T, OutgoingEvent::PacketEvent>)
				{
					if (ENetPeer* peer = m_clients[outEvent.peerId])
					{
						Nz::NetPacket& packet = arg.packet;

						enet_uint32 packetflags = 0;
						if (arg.flags & Nz::ENetPacketFlag_Reliable)
							packetflags |= ENET_PACKET_FLAG_RELIABLE;

						if (arg.flags & Nz::ENetPacketFlag_UnreliableFragment)
							packetflags |= ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;

						if (arg.flags & Nz::ENetPacketFlag_Unsequenced)
							packetflags |= ENET_PACKET_FLAG_UNSEQUENCED;

						enet_peer_send(peer, arg.channelId, enet_packet_create(packet.GetData() + Nz::NetPacket::HeaderSize, packet.GetDataSize(), packetflags));
					}
				}
				else if constexpr (std::is_same_v<T, OutgoingEvent::QueryPeerInfo>)
				{
					if (ENetPeer* peer = m_clients[outEvent.peerId])
					{
						IncomingEvent newEvent;
						newEvent.peerId = m_firstId + outEvent.peerId;

						auto& peerInfo = newEvent.data.emplace<PeerInfo>();
						peerInfo.lastReceiveTime = m_host->serviceTime - peer->lastReceiveTime;
						peerInfo.ping = peer->roundTripTime;

						m_incomingQueue.enqueue(producterToken, std::move(newEvent));
					}
				}
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

			}, outEvent.data);
		}
	}
}
