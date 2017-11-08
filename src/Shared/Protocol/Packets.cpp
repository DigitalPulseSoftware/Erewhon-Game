// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Protocol/Packets.hpp>
#include <Nazara/Core/Algorithm.hpp>
#include <Shared/Utils.hpp>

namespace ewn
{
	namespace Packets
	{
		void Serialize(Nz::NetPacket& packet, const Login& data)
		{
			packet << data.login;
			packet << data.passwordHash;
		}

		void Serialize(Nz::NetPacket& packet, const LoginFailure& data)
		{
			packet << data.reason;
		}

		void Serialize(Nz::NetPacket& packet, const LoginSuccess& data)
		{
		}

		void Unserialize(Nz::NetPacket& packet, Login& data)
		{
			packet >> data.login;
			packet >> data.passwordHash;
		}

		void Unserialize(Nz::NetPacket& packet, LoginFailure& data)
		{
			packet >> data.reason;
		}

		void Unserialize(Nz::NetPacket& packet, LoginSuccess& data)
		{
		}
	}
}
