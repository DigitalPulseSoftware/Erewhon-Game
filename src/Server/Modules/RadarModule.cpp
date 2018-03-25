// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Modules/RadarModule.hpp>
#include <Nazara/Core/Clock.hpp>
#include <Nazara/Math/BoundingVolume.hpp>
#include <Nazara/Physics3D/PhysWorld3D.hpp>
#include <NDK/LuaAPI.hpp>
#include <NDK/World.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent3D.hpp>
#include <NDK/Systems/PhysicsSystem3D.hpp>
#include <Server/Components/RadarComponent.hpp>
#include <Server/Components/SynchronizedComponent.hpp>
#include <Server/Scripting/LuaTypes.hpp>
#include <iostream>

namespace ewn
{
	void RadarModule::ClearLockedTargets()
	{
		RadarComponent& radar = GetSpaceship()->GetComponent<RadarComponent>();
		radar.ClearLockedTargets();
	}

	std::optional<RadarModule::TargetInfo> RadarModule::GetTargetInfo(Ndk::EntityId targetId)
	{
		const Ndk::EntityHandle& spaceship = GetSpaceship();
		Ndk::World* world = spaceship->GetWorld();

		if (!m_entitiesInRadius.Has(targetId))
			return {};

		const Ndk::EntityHandle& targetEntity = world->GetEntity(targetId);

		auto& nodeComponent = targetEntity->GetComponent<Ndk::NodeComponent>();

		TargetInfo targetInfo;
		targetInfo.position = nodeComponent.GetPosition();
		targetInfo.rotation = nodeComponent.GetRotation();

		if (targetEntity->HasComponent<Ndk::PhysicsComponent3D>())
		{
			auto& physComponent = targetEntity->GetComponent<Ndk::PhysicsComponent3D>();
			targetInfo.angularVelocity = physComponent.GetAngularVelocity();
			targetInfo.linearVelocity = physComponent.GetLinearVelocity();
		}
		else
		{
			targetInfo.angularVelocity = Nz::Vector3f::Zero();
			targetInfo.linearVelocity = Nz::Vector3f::Zero();
		}

		return targetInfo;
	}

	bool RadarModule::IsTargetLocked(Ndk::EntityId targetId) const
	{
		RadarComponent& radar = GetSpaceship()->GetComponent<RadarComponent>();
		return radar.IsEntityLocked(targetId);
	}

	bool RadarModule::LockTarget(Ndk::EntityId targetId)
	{
		const Ndk::EntityHandle& spaceship = GetSpaceship();
		Ndk::World* world = spaceship->GetWorld();

		if (!world->IsEntityIdValid(targetId))
			return false;

		if (!m_entitiesInRadius.Has(targetId))
			return false;

		RadarComponent& radar = spaceship->GetComponent<RadarComponent>();
		if (radar.GetLockedEntityCount() + 1 > m_maxLockableTargets)
			return false;

		const Ndk::EntityHandle& targetEntity = world->GetEntity(targetId);

		radar.LockEntity(targetEntity,
		[moduleHandle = CreateHandle()](Ndk::Entity* entity)
		{
			if (!moduleHandle)
				return;

			moduleHandle->RemoveEntityFromRadius(entity);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();

			moduleHandle->PushCallback("OnRadarObjectDestroyed", [id = entity->GetId(), lastPos = nodeComponent.GetPosition()](Nz::LuaState& state)
			{
				state.Push(id);
				state.Push(LuaVec3(lastPos));
				return 2;
			}, false);
		},
		[moduleHandle = CreateHandle()](Ndk::Entity* entity)
		{
			if (!moduleHandle)
				return;

			moduleHandle->RemoveEntityFromRadius(entity);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();

			moduleHandle->PushCallback("OnRadarObjectLeftRange", [id = entity->GetId(), lastPos = nodeComponent.GetPosition()](Nz::LuaState& state)
			{
				state.Push(id);
				state.Push(LuaVec3(lastPos));

				return 2;
			}, false);
		});

		return true;
	}

	void RadarModule::UnlockTarget(Ndk::EntityId targetId)
	{
		const Ndk::EntityHandle& spaceship = GetSpaceship();
		Ndk::World* world = spaceship->GetWorld();

		if (!world->IsEntityIdValid(targetId))
			return;

		const Ndk::EntityHandle& targetEntity = world->GetEntity(targetId);

		RadarComponent& radar = spaceship->GetComponent<RadarComponent>();
		radar.UnlockEntity(targetEntity);
	}

	void RadarModule::Register(Nz::LuaState& lua)
	{
		if (!s_binding)
		{
			s_binding.emplace("Radar");

			s_binding->BindMethod("ClearLockedTargets", &RadarModule::ClearLockedTargets);
			s_binding->BindMethod("EnablePassiveScan", &RadarModule::EnablePassiveScan);
			s_binding->BindMethod("GetTargetInfo", &RadarModule::GetTargetInfo);
			s_binding->BindMethod("IsPassiveScanEnabled", &RadarModule::IsPassiveScanEnabled);
			s_binding->BindMethod("IsTargetLocked", &RadarModule::IsTargetLocked);
			s_binding->BindMethod("LockTarget", &RadarModule::LockTarget);
			s_binding->BindMethod("UnlockTarget", &RadarModule::UnlockTarget);

			// Workaround for value reply bug
			//s_binding->BindMethod("ScanInCone", &RadarModule::ScanInCone);
			s_binding->BindMethod("GetTargetInfo", [](Nz::LuaState& state, RadarModule* radar, std::size_t /*argCount*/)
			{
				int argIndex = 2;
				decltype(auto) result = radar->GetTargetInfo(state.Check<Ndk::EntityId>(&argIndex));
				if (result.has_value())
				{
					state.PushTable(0, 4);
						state.PushField("position", result->position);
						state.PushField("rotation", result->rotation);
						state.PushField("angularVelocity", result->angularVelocity);
						state.PushField("linearVelocity", result->linearVelocity);
				}
				else
					state.PushNil();

				return 1;
			});
		}

		s_binding->Register(lua);

		lua.PushField("Radar", this);
	}

	void RadarModule::Run()
	{
		if (m_isPassiveScanEnabled)
		{
			Nz::UInt64 now = ServerApplication::GetAppTime();
			if (now - m_lastPassiveScanTime > 500)
			{
				PerformScan();
				m_lastPassiveScanTime = now;
			}
		}

		const Ndk::EntityHandle& spaceship = GetSpaceship();
		auto& spaceshipNode = spaceship->GetComponent<Ndk::NodeComponent>();
		auto& spaceshipRadar = spaceship->GetComponent<RadarComponent>();

		spaceshipRadar.CheckTargetRange(spaceshipNode.GetPosition(), m_detectionRadius);
	}

	void RadarModule::Initialize(Ndk::Entity* spaceship)
	{
		spaceship->AddComponent<RadarComponent>();
	}

	void RadarModule::PerformScan()
	{
		const Ndk::EntityHandle& spaceship = GetSpaceship();
		auto& spaceshipNode = spaceship->GetComponent<Ndk::NodeComponent>();
		auto& spaceshipPhys = spaceship->GetComponent<Ndk::PhysicsComponent3D>();

		Nz::Vector3f position = spaceshipNode.GetPosition();
		Nz::Boxf detectionBox = Nz::Boxf(position - Nz::Vector3f(m_detectionRadius), position + Nz::Vector3f(m_detectionRadius));

		Ndk::World* world = spaceship->GetWorld();
		Nz::PhysWorld3D& physWorld = world->GetSystem<Ndk::PhysicsSystem3D>().GetWorld();
		float maxSquaredRadius = m_detectionRadius * m_detectionRadius;
		physWorld.ForEachBodyInAABB(detectionBox, [&](Nz::RigidBody3D& body)
		{
			Nz::Vector3f bodyPosition = body.GetPosition();
			if (bodyPosition.SquaredDistance(position) < maxSquaredRadius)
			{
				Ndk::EntityId bodyId = static_cast<Ndk::EntityId>(reinterpret_cast<std::ptrdiff_t>(body.GetUserdata()));
				if (!m_entitiesInRadius.Has(bodyId) && bodyId != spaceship->GetId())
				{
					const Ndk::EntityHandle& bodyEntity = world->GetEntity(bodyId);
					auto& syncComponent = bodyEntity->GetComponent<SynchronizedComponent>();

					m_entitiesInRadius.Insert(bodyEntity);

					PushCallback("OnRadarNewObjectInRange", [id = bodyId, type = syncComponent.GetType(), bodyPosition](Nz::LuaState& state)
					{
						state.Push(id);
						state.Push(type);
						state.Push(LuaVec3(bodyPosition));
						return 3;
					},
					false);
				}
			}

			return true;
		});
	}

	std::optional<Nz::LuaClass<RadarModuleHandle>> RadarModule::s_binding;
}
