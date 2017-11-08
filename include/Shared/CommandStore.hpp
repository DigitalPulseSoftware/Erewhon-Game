// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_COMMANDSTORE_HPP
#define EREWHON_SHARED_COMMANDSTORE_HPP

#include <Nazara/Network/ENetPacket.hpp>
#include <Nazara/Network/NetPacket.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <functional>
#include <vector>

namespace ewn
{
	class Player;

	class CommandStore
	{
		public:
			struct IncomingCommand;
			struct OutgoingCommand;

			CommandStore() = default;
			~CommandStore();

			template<typename T> const IncomingCommand& GetIncomingCommand() const;
			template<typename T> const OutgoingCommand& GetOutgoingCommand() const;

			template<typename T>
			void SerializePacket(Nz::NetPacket& packet, const T& data) const;

			bool UnserializePacket(std::size_t peerId, Nz::NetPacket&& packet) const;

			using UnserializeFunction = std::function<void(std::size_t peerId, Nz::NetPacket&& packet)>;

			struct IncomingCommand
			{
				bool enabled = false;
				UnserializeFunction unserialize;
				const char* name;
			};

			struct OutgoingCommand
			{
				bool enabled = false;
				const char* name;
				Nz::ENetPacketFlags flags;
				Nz::UInt8 channelId;
			};

		protected:
			template<typename T, typename CB> void RegisterIncomingCommand(const char* name, CB&& callback);
			template<typename T> void RegisterOutgoingCommand(const char* name, Nz::ENetPacketFlags flags, Nz::UInt8 channelId);

		private:
			using HandleFunction = std::function<void(Nz::NetPacket& packet)>;

			std::vector<IncomingCommand> m_incomingCommands;
			std::vector<OutgoingCommand> m_outgoingCommands;
	};
}

#include <Shared/CommandStore.inl>

#endif // EREWHON_SHARED_COMMANDSTORE_HPP