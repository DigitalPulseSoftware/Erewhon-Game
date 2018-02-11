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
			packet << data.stateId;
			packet << data.serverTime;
			packet << data.lastProcessedInputTime;

			packet << Nz::UInt32(data.entities.size());
			for (const auto& entity : data.entities)
			{
				packet << entity.id;
				packet << entity.position;
				packet << entity.rotation;
				packet << entity.angularVelocity;
				packet << entity.linearVelocity;
			}
		}

		void Serialize(Nz::NetPacket& packet, const BotMessage& data)
		{
			packet << Nz::UInt8(data.messageType);
			packet << data.errorMessage;
		}

		void Serialize(Nz::NetPacket& packet, const ChatMessage& data)
		{
			packet << data.message;
		}

		void Serialize(Nz::NetPacket& packet, const ControlEntity& data)
		{
			packet << data.id;
		}

		void Serialize(Nz::NetPacket& packet, const CreateEntity& data)
		{
			packet << data.entityType;
			packet << data.id;
			packet << data.angularVelocity;
			packet << data.linearVelocity;
			packet << data.position;
			packet << data.rotation;
			packet << data.name;
		}

		void Serialize(Nz::NetPacket& packet, const DeleteEntity& data)
		{
			packet << data.id;
		}

		void Serialize(Nz::NetPacket& packet, const IntegrityUpdate& data)
		{
			packet << data.integrityValue;
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
			packet << static_cast<Nz::UInt8>(data.reason);
		}

		void Serialize(Nz::NetPacket& packet, const LoginSuccess& data)
		{
		}

		void Serialize(Nz::NetPacket& packet, const PlayerChat& data)
		{
			packet << data.text;
		}

		void Serialize(Nz::NetPacket & packet, const PlayerMovement& data)
		{
			packet << data.inputTime;
			packet << data.direction;
			packet << data.rotation;
		}

		void Serialize(Nz::NetPacket& packet, const PlayerShoot& data)
		{
		}

		void Serialize(Nz::NetPacket& packet, const Register& data)
		{
			packet << data.login;
			packet << data.email;
			packet << data.passwordHash;
		}

		void Serialize(Nz::NetPacket& packet, const RegisterFailure& data)
		{
			packet << static_cast<Nz::UInt8>(data.reason);
		}

		void Serialize(Nz::NetPacket& packet, const RegisterSuccess& data)
		{
		}

		void Serialize(Nz::NetPacket& packet, const TimeSyncRequest& data)
		{
			packet << data.requestId;
		}

		void Serialize(Nz::NetPacket& packet, const TimeSyncResponse& data)
		{
			packet << data.requestId;
			packet << data.serverTime;
		}

		void Serialize(Nz::NetPacket& packet, const UploadScript& data)
		{
			packet << data.code;
		}

		void Unserialize(Nz::NetPacket& packet, ArenaState& data)
		{
			packet >> data.stateId;
			packet >> data.serverTime;
			packet >> data.lastProcessedInputTime;

			Nz::UInt32 entityCount;
			packet >> entityCount;

			data.entities.resize(entityCount);
			for (auto& entity : data.entities)
			{
				packet >> entity.id;
				packet >> entity.position;
				packet >> entity.rotation;
				packet >> entity.angularVelocity;
				packet >> entity.linearVelocity;
			}
		}

		void Unserialize(Nz::NetPacket& packet, BotMessage& data)
		{
			Nz::UInt8 messageType;
			packet >> messageType;
			data.messageType = static_cast<BotMessageType>(messageType);

			packet >> data.errorMessage;
		}

		void Unserialize(Nz::NetPacket& packet, ChatMessage& data)
		{
			packet >> data.message;
		}

		void Unserialize(Nz::NetPacket& packet, ControlEntity& data)
		{
			packet >> data.id;
		}

		void Unserialize(Nz::NetPacket& packet, CreateEntity& data)
		{
			packet >> data.entityType;
			packet >> data.id;
			packet >> data.angularVelocity;
			packet >> data.linearVelocity;
			packet >> data.position;
			packet >> data.rotation;
			packet >> data.name;
		}

		void Unserialize(Nz::NetPacket& packet, DeleteEntity& data)
		{
			packet >> data.id;
		}

		void Unserialize(Nz::NetPacket& packet, IntegrityUpdate& data)
		{
			packet >> data.integrityValue;
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
			Nz::UInt8 reason;
			packet >> reason;
			data.reason = static_cast<LoginFailureReason>(reason);
		}

		void Unserialize(Nz::NetPacket& packet, LoginSuccess& data)
		{
		}

		void Unserialize(Nz::NetPacket& packet, PlayerChat& data)
		{
			packet >> data.text;
		}

		void Unserialize(Nz::NetPacket& packet, PlayerMovement& data)
		{
			packet >> data.inputTime;
			packet >> data.direction;
			packet >> data.rotation;
		}

		void Unserialize(Nz::NetPacket& packet, PlayerShoot& data)
		{
		}

		void Unserialize(Nz::NetPacket& packet, Register& data)
		{
			packet >> data.login;
			packet >> data.email;
			packet >> data.passwordHash;
		}

		void Unserialize(Nz::NetPacket& packet, RegisterFailure& data)
		{
			Nz::UInt8 reason;
			packet >> reason;
			data.reason = static_cast<RegisterFailureReason>(reason);
		}

		void Unserialize(Nz::NetPacket& packet, RegisterSuccess& data)
		{
		}

		void Unserialize(Nz::NetPacket& packet, TimeSyncRequest& data)
		{
			packet >> data.requestId;
		}

		void Unserialize(Nz::NetPacket& packet, TimeSyncResponse& data)
		{
			packet >> data.requestId;
			packet >> data.serverTime;
		}

		void Unserialize(Nz::NetPacket& packet, UploadScript& data)
		{
			packet >> data.code;
		}
	}
}
