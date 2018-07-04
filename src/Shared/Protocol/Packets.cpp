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
		void Serialize(PacketSerializer& serializer, ArenaList& data)
		{
			serializer.SerializeArraySize(data.arenas);
			for (auto& arenaData : data.arenas)
				serializer &= arenaData.arenaName;
		}

		void Serialize(PacketSerializer& serializer, ArenaPrefabs& data)
		{
			serializer &= data.startId;

			serializer.SerializeArraySize(data.prefabs);
			for (auto& prefabs : data.prefabs)
			{
				serializer.SerializeArraySize(prefabs.models);
				for (auto& model : prefabs.models)
				{
					serializer &= model.modelId;
					serializer &= model.rotation;
					serializer &= model.position;
					serializer &= model.scale;
				}

				serializer.SerializeArraySize(prefabs.sounds);
				for (auto& sound : prefabs.sounds)
				{
					serializer &= sound.soundId;
					serializer &= sound.position;
				}

				serializer.SerializeArraySize(prefabs.visualEffects);
				for (auto& effect : prefabs.visualEffects)
				{
					serializer &= effect.effectNameId;
					serializer &= effect.rotation;
					serializer &= effect.position;
					serializer &= effect.scale;
				}
			}
		}

		void Serialize(PacketSerializer& serializer, ArenaParticleSystems& data)
		{
			serializer &= data.startId;

			serializer.SerializeArraySize(data.particleSystems);
			for (auto& particleSystem : data.particleSystems)
			{
				serializer.SerializeArraySize(particleSystem.particleGroups);
				for (auto& particleGroup : particleSystem.particleGroups)
					serializer &= particleGroup.particleGroupNameId;
			}
		}

		void Serialize(PacketSerializer& serializer, ArenaSounds& data)
		{
			serializer &= data.startId;

			serializer.SerializeArraySize(data.sounds);
			for (auto& sound : data.sounds)
				serializer &= sound.filePath;
		}

		void Serialize(PacketSerializer& serializer, ArenaState& data)
		{
			serializer &= data.stateId;
			serializer &= data.serverTime;
			serializer &= data.lastProcessedInputTime;

			serializer.SerializeArraySize(data.entities);
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

		void Serialize(PacketSerializer& serializer, CreateFleet& data)
		{
			serializer &= data.fleetName;
			serializer.SerializeArraySize(data.spaceshipNames);
			for (auto& name : data.spaceshipNames)
				serializer &= name;

			serializer.SerializeArraySize(data.spaceships);
			for (auto& spaceship : data.spaceships)
			{
				serializer &= spaceship.spaceshipNameId;
				serializer &= spaceship.spaceshipPosition;
			}
		}

		void Serialize(PacketSerializer& serializer, CreateFleetFailure& data)
		{
			serializer.Serialize<Nz::UInt8>(data.reason);
		}

		void Serialize(PacketSerializer& serializer, CreateFleetSuccess& data)
		{
		}

		void Serialize(PacketSerializer& serializer, CreateSpaceship& data)
		{
			serializer &= data.hullId;
			serializer &= data.spaceshipName;
			serializer &= data.spaceshipCode;

			serializer.SerializeArraySize(data.modules);
			for (auto& moduleInfo : data.modules)
			{
				serializer.Serialize<Nz::UInt8>(moduleInfo.type);
				serializer &= moduleInfo.moduleId;
			}
		}

		void Serialize(PacketSerializer& serializer, CreateSpaceshipFailure& data)
		{
			serializer.Serialize<Nz::UInt8>(data.reason);
		}

		void Serialize(PacketSerializer& serializer, CreateSpaceshipSuccess& data)
		{
		}

		void Serialize(PacketSerializer& serializer, DeleteEntity& data)
		{
			serializer &= data.id;
		}

		void Serialize(PacketSerializer& serializer, DeleteFleet& data)
		{
			serializer &= data.fleetName;
		}

		void Serialize(PacketSerializer& serializer, DeleteFleetFailure& data)
		{
			serializer.Serialize<Nz::UInt8>(data.reason);
		}

		void Serialize(PacketSerializer& serializer, DeleteFleetSuccess& data)
		{
		}

		void Serialize(PacketSerializer& serializer, DeleteSpaceship& data)
		{
			serializer &= data.spaceshipName;
		}

		void Serialize(PacketSerializer& serializer, DeleteSpaceshipFailure& data)
		{
			serializer.Serialize<Nz::UInt8>(data.reason);
		}

		void Serialize(PacketSerializer& serializer, DeleteSpaceshipSuccess& data)
		{
		}

		void Serialize(PacketSerializer& serializer, FleetInfo& data)
		{
			serializer.Serialize<Nz::UInt8>(data.spaceshipInfo);
			serializer &= data.fleetName;

			serializer.SerializeArraySize(data.spaceshipTypes);
			for (auto& spaceshipType : data.spaceshipTypes)
			{
				serializer &= spaceshipType.dimensions;
				serializer &= spaceshipType.scale;

				if (data.spaceshipInfo & SpaceshipQueryInfo::Code)
					serializer &= spaceshipType.script;

				if (data.spaceshipInfo & SpaceshipQueryInfo::HullModelPath)
					serializer &= spaceshipType.hullModelPath;

				if (data.spaceshipInfo & SpaceshipQueryInfo::Modules)
				{
					serializer.SerializeArraySize(spaceshipType.modules);
					for (auto& moduleData : spaceshipType.modules)
					{
						serializer &= moduleData.currentModule;
						serializer.Serialize<Nz::UInt8>(moduleData.type);
					}
				}

				if (data.spaceshipInfo & SpaceshipQueryInfo::Name)
					serializer &= spaceshipType.name;
			}

			serializer.SerializeArraySize(data.spaceships);
			for (auto& spaceship : data.spaceships)
			{
				serializer &= spaceship.position;
				serializer &= spaceship.spaceshipType;
			}
		}

		void Serialize(PacketSerializer& serializer, FleetList& data)
		{
			serializer.SerializeArraySize(data.fleets);
			for (auto& fleet : data.fleets)
				serializer &= fleet.name;
		}

		void Serialize(PacketSerializer& serializer, HullList& data)
		{
			serializer.SerializeArraySize(data.hulls);
			for (auto& hullInfo : data.hulls)
			{
				serializer &= hullInfo.hullId;
				serializer &= hullInfo.hullModelPathId;
				serializer &= hullInfo.name;
				serializer &= hullInfo.description;

				serializer.SerializeArraySize(hullInfo.slots);
				for (auto& slotInfo : hullInfo.slots)
					serializer.Serialize<Nz::UInt8>(slotInfo.type);
			}
		}

		void Serialize(PacketSerializer& serializer, InstantiateParticleSystem& data)
		{
			serializer &= data.particleSystemId;
			serializer &= data.rotation;
			serializer &= data.position;
			serializer &= data.scale;
		}

		void Serialize(PacketSerializer& serializer, IntegrityUpdate& data)
		{
			serializer &= data.integrityValue;
		}

		void Serialize(PacketSerializer & serializer, LeaveArena& data)
		{
		}

		void Serialize(PacketSerializer& serializer, JoinArena& data)
		{
			serializer &= data.arenaIndex;
		}

		void Serialize(PacketSerializer& serializer, Login& data)
		{
			serializer &= data.login;
			serializer &= data.passwordHash;

			serializer.Serialize<Nz::UInt8>(data.generateConnectionToken);
		}

		void Serialize(PacketSerializer& serializer, LoginByToken& data)
		{
			serializer.SerializeArraySize(data.connectionToken);
			for (auto& data : data.connectionToken)
				serializer &= data;

			serializer.Serialize<Nz::UInt8>(data.generateConnectionToken);
		}

		void Serialize(PacketSerializer& serializer, LoginFailure& data)
		{
			serializer.Serialize<Nz::UInt8>(data.reason);
		}

		void Serialize(PacketSerializer& serializer, LoginSuccess& data)
		{
			serializer.SerializeArraySize(data.connectionToken);
			for (auto& data : data.connectionToken)
				serializer &= data;
		}

		void Serialize(PacketSerializer& serializer, ModuleList& data)
		{
			// Modules
			serializer.SerializeArraySize(data.modules);
			for (auto& moduleTypeInfo : data.modules)
			{
				serializer.Serialize<Nz::UInt8>(moduleTypeInfo.type);

				// Available modules
				serializer.SerializeArraySize(moduleTypeInfo.availableModules);
				for (auto& moduleInfo : moduleTypeInfo.availableModules)
				{
					serializer &= moduleInfo.moduleId;
					serializer &= moduleInfo.moduleName;
				}
			}
		}

		void Serialize(PacketSerializer& serializer, NetworkStrings& data)
		{
			serializer &= data.startId;

			serializer.SerializeArraySize(data.strings);
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

		void Serialize(PacketSerializer& serializer, QueryArenaList& data)
		{
		}

		void Serialize(PacketSerializer& serializer, QueryFleetInfo& data)
		{
			serializer.Serialize<Nz::UInt8>(data.spaceshipInfo);
			serializer &= data.fleetName;
		}

		void Serialize(PacketSerializer& serializer, QueryFleetList& data)
		{
		}

		void Serialize(PacketSerializer& serializer, QueryHullList& data)
		{
		}

		void Serialize(PacketSerializer& serializer, QueryModuleList& data)
		{
		}

		void Serialize(PacketSerializer& serializer, QuerySpaceshipInfo& data)
		{
			serializer.Serialize<Nz::UInt8>(data.info);
			serializer &= data.spaceshipName;
		}

		void Serialize(PacketSerializer& serializer, QuerySpaceshipList& data)
		{
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

		void Serialize(PacketSerializer& serializer, SpaceshipInfo& data)
		{
			serializer.Serialize<Nz::UInt8>(data.info);
			serializer &= data.collisionBox;
			serializer &= data.hullId;
			serializer &= data.scale;

			if (data.info & SpaceshipQueryInfo::Code)
				serializer &= data.code;

			if (data.info & SpaceshipQueryInfo::HullModelPath)
				serializer &= data.hullModelPath;

			if (data.info & SpaceshipQueryInfo::Name)
				serializer &= data.spaceshipName;

			if (data.info & SpaceshipQueryInfo::Modules)
			{
				serializer.SerializeArraySize(data.modules);
				for (auto& moduleInfo : data.modules)
				{
					serializer &= moduleInfo.currentModule;
					serializer.Serialize<Nz::UInt8>(moduleInfo.type);
				}
			}
		}

		void Serialize(PacketSerializer& serializer, SpaceshipList& data)
		{
			serializer.SerializeArraySize(data.spaceships);
			for (auto& spaceship : data.spaceships)
				serializer &= spaceship.name;
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

		void Serialize(PacketSerializer& serializer, UpdateFleet& data)
		{
			serializer &= data.fleetName;
			serializer &= data.newFleetName;
			serializer.SerializeArraySize(data.spaceshipNames);
			for (auto& name : data.spaceshipNames)
				serializer &= name;

			serializer.SerializeArraySize(data.spaceships);
			for (auto& spaceship : data.spaceships)
			{
				serializer &= spaceship.spaceshipNameId;
				serializer &= spaceship.spaceshipPosition;
			}
		}

		void Serialize(PacketSerializer& serializer, UpdateFleetFailure& data)
		{
			serializer.Serialize<Nz::UInt8>(data.reason);
		}

		void Serialize(PacketSerializer& serializer, UpdateFleetSuccess& data)
		{
		}

		void Serialize(PacketSerializer& serializer, UpdateSpaceship& data)
		{
			serializer &= data.spaceshipName;
			serializer &= data.newSpaceshipName;
			serializer &= data.newSpaceshipCode;

			serializer.SerializeArraySize(data.modifiedModules);
			for (auto& moduleInfo : data.modifiedModules)
			{
				serializer &= moduleInfo.moduleName;
				serializer &= moduleInfo.oldModuleName;
				serializer.Serialize<Nz::UInt8>(moduleInfo.type);
			}
		}

		void Serialize(PacketSerializer& serializer, UpdateSpaceshipFailure& data)
		{
			serializer.Serialize<Nz::UInt8>(data.reason);
		}

		void Serialize(PacketSerializer& serializer, UpdateSpaceshipSuccess& data)
		{
		}
	}
}
