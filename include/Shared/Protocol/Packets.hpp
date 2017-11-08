// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_NETWORK_PACKETS_HPP
#define EREWHON_SHARED_NETWORK_PACKETS_HPP

#include <Nazara/Prerequesites.hpp>
#include <Nazara/Network/NetPacket.hpp>
#include <array>
#include <variant>

namespace ewn
{
	enum class PacketType
	{
		Login,
		LoginFailure,
		LoginSuccess
	};

	template<PacketType PT> struct PacketTag
	{
		static constexpr PacketType Type = PT;
	};

	namespace Packets
	{
#define DeclarePacket(Type) struct Type : PacketTag<PacketType:: Type >

		DeclarePacket(Login)
		{
			std::string login;
			std::string passwordHash;
		};

		DeclarePacket(LoginFailure)
		{
			Nz::UInt8 reason;
		};

		DeclarePacket(LoginSuccess)
		{
		};

#undef DeclarePacket

		void Serialize(Nz::NetPacket& packet, const Login& data);
		void Serialize(Nz::NetPacket& packet, const LoginFailure& data);
		void Serialize(Nz::NetPacket& packet, const LoginSuccess& data);
		void Unserialize(Nz::NetPacket& packet, Login& data);
		void Unserialize(Nz::NetPacket& packet, LoginFailure& data);
		void Unserialize(Nz::NetPacket& packet, LoginSuccess& data);
	}
}

#include <Shared/Protocol/Packets.inl>

#endif // EREWHON_SHARED_NETWORK_PACKETS_HPP