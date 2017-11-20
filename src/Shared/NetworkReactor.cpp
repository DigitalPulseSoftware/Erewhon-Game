// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/NetworkReactor.hpp>
#include <Shared/Config.hpp>
#include <Shared/Utils.hpp>
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <stdexcept>

namespace ewn
{
	NetworkReactor::NetworkReactor(std::size_t firstId, const Nz::IpAddress& listenAddress, std::size_t maxClient) :
	m_firstId(firstId)
	{
		if (!m_host.Create(listenAddress, maxClient, NetworkChannelCount))
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
			newClientId = peerId;
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

	void NetworkReactor::SendData(std::size_t peerId, Nz::UInt8 channelId, Nz::ENetPacketFlags flags, Nz::NetPacket&& packet)
	{
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
			Nz::ENetPeer* peer = m_host.Connect(request.remoteAddress, NetworkChannelCount, request.data);
			Nz::UInt16 peerId = peer->GetPeerId();
			m_clients[peerId] = peer;

			request.callback(peerId);
		}
	}

	void NetworkReactor::ReceivePackets(const moodycamel::ProducerToken& producterToken)
	{
		Nz::ENetEvent event;
		if (m_host.Service(&event, 5) > 0)
		{
			do
			{
				switch (event.type)
				{
					case Nz::ENetEventType::Disconnect:
					{
						Nz::UInt16 peerId = event.peer->GetPeerId();
						m_clients[peerId] = nullptr;

						IncomingEvent::DisconnectEvent disconnectEvent;
						disconnectEvent.data = event.data;

						IncomingEvent newEvent;
						newEvent.peerId = m_firstId + peerId;
						newEvent.data.emplace<IncomingEvent::DisconnectEvent>(std::move(disconnectEvent));

						m_incomingQueue.enqueue(producterToken, std::move(newEvent));
						break;
					}

					case Nz::ENetEventType::IncomingConnect:
					case Nz::ENetEventType::OutgoingConnect:
					{
						Nz::UInt16 peerId = event.peer->GetPeerId();
						m_clients[peerId] = event.peer;

						IncomingEvent::ConnectEvent connectEvent;
						connectEvent.data = event.data;
						connectEvent.outgoingConnection = (event.type == Nz::ENetEventType::OutgoingConnect);

						IncomingEvent newEvent;
						newEvent.peerId = m_firstId + peerId;
						newEvent.data.emplace<IncomingEvent::ConnectEvent>(std::move(connectEvent));

						m_incomingQueue.enqueue(producterToken, std::move(newEvent));
						break;
					}

					case Nz::ENetEventType::Receive:
					{
						Nz::UInt16 peerId = event.peer->GetPeerId();

						IncomingEvent::PacketEvent packetEvent;
						packetEvent.packet = std::move(event.packet->data);

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
			while (m_host.CheckEvents(&event));
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
					if (Nz::ENetPeer* peer = m_clients[outEvent.peerId])
					{
						switch (arg.type)
						{
							case DisconnectionType::Kick:
							{
								peer->DisconnectNow(arg.data);

								// DisconnectNow does not generate Disconnect event
								m_clients[outEvent.peerId] = nullptr;

								IncomingEvent::DisconnectEvent disconnectEvent{};
								disconnectEvent.data = 0;

								IncomingEvent newEvent{};
								newEvent.peerId = m_firstId + outEvent.peerId;
								newEvent.data.emplace<IncomingEvent::DisconnectEvent>(std::move(disconnectEvent));

								m_incomingQueue.enqueue(producterToken, std::move(newEvent));
								break;
							}

							case DisconnectionType::Later:
								peer->DisconnectLater(arg.data);
								break;

							case DisconnectionType::Normal:
								peer->Disconnect(arg.data);
								break;

							default:
								assert(!"Unknown disconnection type");
								break;
						}
					}
				}
				else if constexpr (std::is_same_v<T, OutgoingEvent::PacketEvent>)
				{
					if (Nz::ENetPeer* peer = m_clients[outEvent.peerId])
						peer->Send(arg.channelId, arg.flags, std::move(arg.packet));
				}
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

			}, outEvent.data);
		}
	}
}
