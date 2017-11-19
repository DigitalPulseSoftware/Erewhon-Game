// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Protocol/Packets.hpp>
#include <Nazara/Core/Algorithm.hpp>
#include <Nazara/Math/Vector3.hpp>
#include <Shared/Utils.hpp>

namespace ewn
{
	namespace Packets
	{
		void Serialize(Nz::NetPacket& packet, const ArenaState& data)
		{
			packet << Nz::UInt32(data.spaceships.size());
			for (const auto& spaceship : data.spaceships)
			{
				packet << spaceship.id;
				packet << spaceship.position;
				packet << spaceship.rotation;
			}
		}

		void Serialize(Nz::NetPacket& packet, const ControlSpaceship& data)
		{
			packet << data.id;
		}

		void Serialize(Nz::NetPacket& packet, const CreateSpaceship& data)
		{
			packet << data.id;
			packet << data.position;
			packet << data.rotation;
			packet << data.name;
		}

		void Serialize(Nz::NetPacket& packet, const DeleteSpaceship& data)
		{
			packet << data.id;
		}

		void Serialize(Nz::NetPacket& packet, const JoinArena& data)
		{
		}

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

		void Serialize(Nz::NetPacket & packet, const PlayerMovement& data)
		{
			packet << data.direction;
			packet << data.rotation;
		}

		void Unserialize(Nz::NetPacket & packet, ArenaState& data)
		{
			Nz::UInt32 spaceshipSize;
			packet >> spaceshipSize;

			data.spaceships.resize(spaceshipSize);
			for (auto& spaceship : data.spaceships)
			{
				packet >> spaceship.id;
				packet >> spaceship.position;
				packet >> spaceship.rotation;
			}
		}

		void Unserialize(Nz::NetPacket& packet, ControlSpaceship& data)
		{
			packet >> data.id;
		}

		void Unserialize(Nz::NetPacket& packet, CreateSpaceship& data)
		{
			packet >> data.id;
			packet >> data.position;
			packet >> data.rotation;
			packet >> data.name;
		}

		void Unserialize(Nz::NetPacket& packet, DeleteSpaceship& data)
		{
			packet >> data.id;
		}

		void Unserialize(Nz::NetPacket& packet, JoinArena& data)
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

		void Unserialize(Nz::NetPacket& packet, PlayerMovement& data)
		{
			packet >> data.direction;
			packet >> data.rotation;
		}
	}
}
