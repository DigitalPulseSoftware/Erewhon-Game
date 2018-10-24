// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_NETWORK_PACKETSERIALIZER_HPP
#define EREWHON_SHARED_NETWORK_PACKETSERIALIZER_HPP

#include <Nazara/Network/NetPacket.hpp>

namespace ewn
{
	class PacketSerializer
	{
		public:
			inline PacketSerializer(Nz::NetPacket& packetBuffer, bool isWriting);
			~PacketSerializer() = default;

			inline bool IsWriting() const;

			template<typename DataType> void Serialize(DataType& data);
			template<typename DataType> void Serialize(const DataType& data) const;
			template<typename PacketType, typename DataType> void Serialize(DataType& data);
			template<typename PacketType, typename DataType> void Serialize(const DataType& data) const;

			template<typename T> void SerializeArraySize(T& array);

			template<typename DataType> void operator&=(DataType& data);
			template<typename DataType> void operator&=(const DataType& data) const;

		private:
			Nz::NetPacket& m_buffer;
			bool m_isWriting;
	};
}

#include <Shared/Protocol/PacketSerializer.inl>

#endif // EREWHON_SHARED_NETWORK_PACKETSERIALIZER_HPP
