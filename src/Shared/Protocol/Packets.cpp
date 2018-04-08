// Copyright (C) 2018 Jérôme Leclercq
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
		void Serialize(PacketSerializer& serializer, ArenaPrefabs& data)
		{
			serializer &= data.startId;

			CompressedUnsigned<Nz::UInt32> prefabCount;
			if (serializer.IsWriting())
				prefabCount = Nz::UInt32(data.prefabs.size());

			serializer &= prefabCount;
			if (!serializer.IsWriting())
				data.prefabs.resize(prefabCount);

			for (auto& prefabs : data.prefabs)
			{
				CompressedUnsigned<Nz::UInt32> modelCount;
				CompressedUnsigned<Nz::UInt32> soundCount;
				CompressedUnsigned<Nz::UInt32> visualEffectCount;
				if (serializer.IsWriting())
				{
					modelCount = Nz::UInt32(prefabs.models.size());
					soundCount = Nz::UInt32(prefabs.sounds.size());
					visualEffectCount = Nz::UInt32(prefabs.visualEffects.size());
				}

				serializer &= modelCount;
				serializer &= soundCount;
				serializer &= visualEffectCount;

				if (!serializer.IsWriting())
				{
					prefabs.models.resize(modelCount);
					prefabs.sounds.resize(soundCount);
					prefabs.visualEffects.resize(visualEffectCount);
				}

				for (auto& model : prefabs.models)
				{
					serializer &= model.modelId;
					serializer &= model.rotation;
					serializer &= model.position;
					serializer &= model.scale;
				}

				for (auto& sound : prefabs.sounds)
				{
					serializer &= sound.soundId;
					serializer &= sound.position;
				}

				for (auto& effect : prefabs.visualEffects)
				{
					serializer &= effect.effectNameId;
					serializer &= effect.rotation;
					serializer &= effect.position;
					serializer &= effect.scale;
				}
			}
		}

		void Serialize(PacketSerializer& serializer, ArenaSounds& data)
		{
			serializer &= data.startId;

			CompressedUnsigned<Nz::UInt32> soundCount;
			if (serializer.IsWriting())
				soundCount = Nz::UInt32(data.sounds.size());

			serializer &= soundCount;
			if (!serializer.IsWriting())
				data.sounds.resize(soundCount);

			for (auto& sound : data.sounds)
				serializer &= sound.filePath;
		}

		void Serialize(PacketSerializer& serializer, ArenaState& data)
		{
			serializer &= data.stateId;
			serializer &= data.serverTime;
			serializer &= data.lastProcessedInputTime;

			CompressedUnsigned<Nz::UInt32> entityCount;
			if (serializer.IsWriting())
				entityCount = Nz::UInt32(data.entities.size());

			serializer &= entityCount;
			if (!serializer.IsWriting())
				data.entities.resize(entityCount);

			for (auto& entity : data.entities)
			{
				serializer &= entity.id;
				serializer &= entity.position;
				serializer &= entity.rotation;
				serializer &= entity.angularVelocity;
				serializer &= entity.linearVelocity;
			}
		}

		void Serialize(PacketSerializer& serializer, BotMessage& data)
		{
			serializer.Serialize<Nz::UInt8>(data.messageType);
			serializer &= data.errorMessage;
		}

		void Serialize(PacketSerializer& serializer, ChatMessage& data)
		{
			serializer &= data.message;
		}

		void Serialize(PacketSerializer& serializer, ControlEntity& data)
		{
			serializer &= data.id;
		}

		void Serialize(PacketSerializer& serializer, CreateEntity& data)
		{
			serializer &= data.angularVelocity;
			serializer &= data.entityId;
			serializer &= data.linearVelocity;
			serializer &= data.position;
			serializer &= data.prefabId;
			serializer &= data.rotation;
			serializer &= data.visualName;
		}

		void Serialize(PacketSerializer& serializer, CreateSpaceship& data)
		{
			serializer &= data.spaceshipName;
			serializer &= data.code;
		}

		void Serialize(PacketSerializer& serializer, DeleteEntity& data)
		{
			serializer &= data.id;
		}

		void Serialize(PacketSerializer& serializer, DeleteSpaceship& data)
		{
			serializer &= data.spaceshipName;
		}

		void Serialize(PacketSerializer& serializer, IntegrityUpdate& data)
		{
			serializer &= data.integrityValue;
		}

		void Serialize(PacketSerializer& serializer, JoinArena& data)
		{
			serializer &= data.arenaIndex;
		}

		void Serialize(PacketSerializer& serializer, Login& data)
		{
			serializer &= data.login;
			serializer &= data.passwordHash;
		}

		void Serialize(PacketSerializer& serializer, LoginFailure& data)
		{
			serializer.Serialize<Nz::UInt8>(data.reason);
		}

		void Serialize(PacketSerializer& serializer, LoginSuccess& data)
		{
		}

		void Serialize(PacketSerializer& serializer, NetworkStrings& data)
		{
			serializer &= data.startId;

			CompressedUnsigned<Nz::UInt32> stringCount;
			if (serializer.IsWriting())
				stringCount = Nz::UInt32(data.strings.size());

			serializer &= stringCount;
			if (!serializer.IsWriting())
				data.strings.resize(stringCount);

			for (auto& string : data.strings)
				serializer &= string;
		}

		void Serialize(PacketSerializer& serializer, PlayerChat& data)
		{
			serializer &= data.text;
		}

		void Serialize(PacketSerializer& serializer, PlayerMovement& data)
		{
			serializer &= data.inputTime;
			serializer &= data.direction;
			serializer &= data.rotation;
		}

		void Serialize(PacketSerializer& serializer, PlayerShoot& data)
		{
		}

		void Serialize(PacketSerializer& serializer, PlaySound& data)
		{
			serializer &= data.soundId;
			serializer &= data.position;
		}

		void Serialize(PacketSerializer& serializer, Register& data)
		{
			serializer &= data.login;
			serializer &= data.email;
			serializer &= data.passwordHash;
		}

		void Serialize(PacketSerializer& serializer, RegisterFailure& data)
		{
			serializer.Serialize<Nz::UInt8>(data.reason);
		}

		void Serialize(PacketSerializer& serializer, RegisterSuccess& data)
		{
		}

		void Serialize(PacketSerializer& serializer, SpawnSpaceship& data)
		{
			serializer &= data.spaceshipName;
		}

		void Serialize(PacketSerializer& serializer, TimeSyncRequest& data)
		{
			serializer &= data.requestId;
		}

		void Serialize(PacketSerializer& serializer, TimeSyncResponse& data)
		{
			serializer &= data.requestId;
			serializer &= data.serverTime;
		}
	}
}
