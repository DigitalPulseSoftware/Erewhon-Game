// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_NETWORK_PACKETS_HPP
#define EREWHON_SHARED_NETWORK_PACKETS_HPP

#include <Shared/Enums.hpp>
#include <Shared/Protocol/CompressedInteger.hpp>
#include <Shared/Protocol/PacketSerializer.hpp>
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
		ArenaModels,
		ArenaPrefabs,
		ArenaState,
		BotMessage,
		ChatMessage,
		CreateSpaceship,
		ControlEntity,
		CreateEntity,
		DeleteEntity,
		DeleteSpaceship,
		IntegrityUpdate,
		JoinArena,
		Login,
		LoginFailure,
		LoginSuccess,
		NetworkStrings,
		PlayerChat,
		PlayerMovement,
		PlayerShoot,
		Register,
		RegisterFailure,
		RegisterSuccess,
		SpawnSpaceship,
		TimeSyncRequest,
		TimeSyncResponse
	};

	template<PacketType PT> struct PacketTag
	{
		static constexpr PacketType Type = PT;
	};

	namespace Packets
	{
#define DeclarePacket(Type) struct Type : PacketTag<PacketType:: Type >

		DeclarePacket(ArenaModels)
		{
			CompressedUnsigned<Nz::UInt32> startId;

			struct Model
			{
				struct Piece
				{
					CompressedUnsigned<Nz::UInt32> modelId;
					Nz::Quaternionf rotation;
					Nz::Vector3f position;
					Nz::Vector3f scale;
				};

				std::vector<Piece> pieces;
			};

			std::vector<Model> models;
		};

		DeclarePacket(ArenaPrefabs)
		{
			CompressedUnsigned<Nz::UInt32> startId;

			struct Prefab
			{
				CompressedUnsigned<Nz::UInt32> visualEffectId;
				CompressedUnsigned<Nz::UInt32> collisionMeshId;
			};

			std::vector<Prefab> prefabs;
		};

		DeclarePacket(ArenaState)
		{
			struct Entity
			{
				CompressedUnsigned<Nz::UInt32> id;
				Nz::Vector3f angularVelocity;
				Nz::Vector3f linearVelocity;
				Nz::Vector3f position;
				Nz::Quaternionf rotation;
			};

			Nz::UInt16 stateId;
			CompressedUnsigned<Nz::UInt64> serverTime;
			CompressedUnsigned<Nz::UInt64> lastProcessedInputTime;
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
			CompressedUnsigned<Nz::UInt32> id;
		};

		DeclarePacket(CreateEntity)
		{
			CompressedUnsigned<Nz::UInt32> entityId;
			CompressedUnsigned<Nz::UInt32> prefabId;
			Nz::Quaternionf rotation;
			Nz::Vector3f angularVelocity;
			Nz::Vector3f linearVelocity;
			Nz::Vector3f position;
			Nz::String visualName;
		};

		DeclarePacket(CreateSpaceship)
		{
			std::string spaceshipName;
			std::string code;
		};

		DeclarePacket(DeleteEntity)
		{
			CompressedUnsigned<Nz::UInt32> id;
		};

		DeclarePacket(DeleteSpaceship)
		{
			std::string spaceshipName;
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

		DeclarePacket(NetworkStrings)
		{
			CompressedUnsigned<Nz::UInt32> startId;
			std::vector<std::string> strings;
		};

		DeclarePacket(PlayerChat)
		{
			std::string text;
		};

		DeclarePacket(PlayerMovement)
		{
			CompressedUnsigned<Nz::UInt64> inputTime; //< Server time
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

		DeclarePacket(SpawnSpaceship)
		{
			std::string spaceshipName;
		};

		DeclarePacket(TimeSyncRequest)
		{
			Nz::UInt8 requestId;
		};

		DeclarePacket(TimeSyncResponse)
		{
			Nz::UInt8 requestId;
			CompressedUnsigned<Nz::UInt64> serverTime;
		};

#undef DeclarePacket

		void Serialize(PacketSerializer& serializer, ArenaModels& data);
		void Serialize(PacketSerializer& serializer, ArenaPrefabs& data);
		void Serialize(PacketSerializer& serializer, ArenaState& data);
		void Serialize(PacketSerializer& serializer, BotMessage& data);
		void Serialize(PacketSerializer& serializer, ChatMessage& data);
		void Serialize(PacketSerializer& serializer, ControlEntity& data);
		void Serialize(PacketSerializer& serializer, CreateEntity& data);
		void Serialize(PacketSerializer& serializer, CreateSpaceship& data);
		void Serialize(PacketSerializer& serializer, DeleteEntity& data);
		void Serialize(PacketSerializer& serializer, DeleteSpaceship& data);
		void Serialize(PacketSerializer& serializer, IntegrityUpdate& data);
		void Serialize(PacketSerializer& serializer, JoinArena& data);
		void Serialize(PacketSerializer& serializer, Login& data);
		void Serialize(PacketSerializer& serializer, LoginFailure& data);
		void Serialize(PacketSerializer& serializer, LoginSuccess& data);
		void Serialize(PacketSerializer& serializer, NetworkStrings& data);
		void Serialize(PacketSerializer& serializer, PlayerChat& data);
		void Serialize(PacketSerializer& serializer, PlayerMovement& data);
		void Serialize(PacketSerializer& serializer, PlayerShoot& data);
		void Serialize(PacketSerializer& serializer, Register& data);
		void Serialize(PacketSerializer& serializer, RegisterFailure& data);
		void Serialize(PacketSerializer& serializer, RegisterSuccess& data);
		void Serialize(PacketSerializer& serializer, SpawnSpaceship& data);
		void Serialize(PacketSerializer& serializer, TimeSyncRequest& data);
		void Serialize(PacketSerializer& serializer, TimeSyncResponse& data);
	}
}

#include <Shared/Protocol/Packets.inl>

#endif // EREWHON_SHARED_NETWORK_PACKETS_HPP
