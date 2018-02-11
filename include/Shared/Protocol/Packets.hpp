// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_NETWORK_PACKETS_HPP
#define EREWHON_SHARED_NETWORK_PACKETS_HPP

#include <Shared/Enums.hpp>
#include <Nazara/Prerequisites.hpp>
#include <Nazara/Core/String.hpp>
#include <Nazara/Math/Quaternion.hpp>
#include <Nazara/Math/Vector3.hpp>
#include <Nazara/Network/NetPacket.hpp>
#include <array>
#include <variant>
#include <vector>

namespace ewn
{
	enum class PacketType
	{
		ArenaState,
		BotMessage,
		ChatMessage,
		ControlEntity,
		CreateEntity,
		DeleteEntity,
		IntegrityUpdate,
		JoinArena,
		Login,
		LoginFailure,
		LoginSuccess,
		PlayerChat,
		PlayerMovement,
		PlayerShoot,
		Register,
		RegisterFailure,
		RegisterSuccess,
		TimeSyncRequest,
		TimeSyncResponse,
		UploadScript
	};

	template<PacketType PT> struct PacketTag
	{
		static constexpr PacketType Type = PT;
	};

	namespace Packets
	{
#define DeclarePacket(Type) struct Type : PacketTag<PacketType:: Type >

		DeclarePacket(ArenaState)
		{
			struct Entity
			{
				Nz::UInt32 id;
				Nz::Vector3f angularVelocity;
				Nz::Vector3f linearVelocity;
				Nz::Vector3f position;
				Nz::Quaternionf rotation;
			};

			Nz::UInt16 stateId;
			Nz::UInt64 serverTime;
			Nz::UInt64 lastProcessedInputTime;
			std::vector<Entity> entities;
		};

		DeclarePacket(BotMessage)
		{
			BotMessageType messageType;
			std::string errorMessage;
		};

		DeclarePacket(ChatMessage)
		{
			std::string message;
		};

		DeclarePacket(ControlEntity)
		{
			Nz::UInt32 id;
		};

		DeclarePacket(CreateEntity)
		{
			Nz::UInt32 id;
			Nz::Vector3f angularVelocity;
			Nz::Vector3f linearVelocity;
			Nz::Vector3f position;
			Nz::Quaternionf rotation;
			Nz::String name;
			Nz::String entityType;
		};

		DeclarePacket(DeleteEntity)
		{
			Nz::UInt32 id;
		};

		DeclarePacket(IntegrityUpdate)
		{
			Nz::UInt8 integrityValue;
		};

		DeclarePacket(JoinArena)
		{
		};

		DeclarePacket(Login)
		{
			std::string login;
			std::string passwordHash;
		};

		DeclarePacket(LoginFailure)
		{
			LoginFailureReason reason;
		};

		DeclarePacket(LoginSuccess)
		{
		};

		DeclarePacket(PlayerChat)
		{
			std::string text;
		};

		DeclarePacket(PlayerMovement)
		{
			Nz::UInt64 inputTime; //< Server time
			Nz::Vector3f direction;
			Nz::Vector3f rotation;
		};

		DeclarePacket(PlayerShoot)
		{
		};

		DeclarePacket(Register)
		{
			std::string login;
			std::string email;
			std::string passwordHash;
		};

		DeclarePacket(RegisterFailure)
		{
			RegisterFailureReason reason;
		};

		DeclarePacket(RegisterSuccess)
		{
		};

		DeclarePacket(TimeSyncRequest)
		{
			Nz::UInt8 requestId;
		};

		DeclarePacket(TimeSyncResponse)
		{
			Nz::UInt8 requestId;
			Nz::UInt64 serverTime;
		};

		DeclarePacket(UploadScript)
		{
			std::string code;
		};

#undef DeclarePacket

		void Serialize(Nz::NetPacket& packet, const ArenaState& data);
		void Serialize(Nz::NetPacket& packet, const BotMessage& data);
		void Serialize(Nz::NetPacket& packet, const ChatMessage& data);
		void Serialize(Nz::NetPacket& packet, const ControlEntity& data);
		void Serialize(Nz::NetPacket& packet, const CreateEntity& data);
		void Serialize(Nz::NetPacket& packet, const DeleteEntity& data);
		void Serialize(Nz::NetPacket& packet, const IntegrityUpdate& data);
		void Serialize(Nz::NetPacket& packet, const JoinArena& data);
		void Serialize(Nz::NetPacket& packet, const Login& data);
		void Serialize(Nz::NetPacket& packet, const LoginFailure& data);
		void Serialize(Nz::NetPacket& packet, const LoginSuccess& data);
		void Serialize(Nz::NetPacket& packet, const PlayerChat& data);
		void Serialize(Nz::NetPacket& packet, const PlayerMovement& data);
		void Serialize(Nz::NetPacket& packet, const PlayerShoot& data);
		void Serialize(Nz::NetPacket& packet, const Register& data);
		void Serialize(Nz::NetPacket& packet, const RegisterFailure& data);
		void Serialize(Nz::NetPacket& packet, const RegisterSuccess& data);
		void Serialize(Nz::NetPacket& packet, const TimeSyncRequest& data);
		void Serialize(Nz::NetPacket& packet, const TimeSyncResponse& data);
		void Serialize(Nz::NetPacket& packet, const UploadScript& data);

		void Unserialize(Nz::NetPacket& packet, ArenaState& data);
		void Unserialize(Nz::NetPacket& packet, BotMessage& data);
		void Unserialize(Nz::NetPacket& packet, ChatMessage& data);
		void Unserialize(Nz::NetPacket& packet, ControlEntity& data);
		void Unserialize(Nz::NetPacket& packet, CreateEntity& data);
		void Unserialize(Nz::NetPacket& packet, DeleteEntity& data);
		void Unserialize(Nz::NetPacket& packet, IntegrityUpdate& data);
		void Unserialize(Nz::NetPacket& packet, JoinArena& data);
		void Unserialize(Nz::NetPacket& packet, Login& data);
		void Unserialize(Nz::NetPacket& packet, LoginFailure& data);
		void Unserialize(Nz::NetPacket& packet, LoginSuccess& data);
		void Unserialize(Nz::NetPacket& packet, PlayerChat& data);
		void Unserialize(Nz::NetPacket& packet, PlayerMovement& data);
		void Unserialize(Nz::NetPacket& packet, PlayerShoot& data);
		void Unserialize(Nz::NetPacket& packet, Register& data);
		void Unserialize(Nz::NetPacket& packet, RegisterFailure& data);
		void Unserialize(Nz::NetPacket& packet, RegisterSuccess& data);
		void Unserialize(Nz::NetPacket& packet, TimeSyncRequest& data);
		void Unserialize(Nz::NetPacket& packet, TimeSyncResponse& data);
		void Unserialize(Nz::NetPacket& packet, UploadScript& data);
	}
}

#include <Shared/Protocol/Packets.inl>

#endif // EREWHON_SHARED_NETWORK_PACKETS_HPP
