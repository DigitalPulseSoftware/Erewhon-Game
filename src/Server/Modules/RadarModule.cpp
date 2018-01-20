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
#include <Server/Components/SynchronizedComponent.hpp>
#include <Server/Scripting/LuaTypes.hpp>
#include <iostream>

namespace ewn
{
	bool RadarModule::IsConeScanReady() const
	{
		Nz::UInt64 currentTime = Nz::GetElapsedMilliseconds();
		return currentTime - m_lastConeScanTime >= 500;
	}

	bool RadarModule::IsTargetScanReady() const
	{
		Nz::UInt64 currentTime = Nz::GetElapsedMilliseconds();
		return currentTime - m_lastTargetScanTime >= 100;
	}

	std::optional<RadarModule::ConeScanResults> RadarModule::ScanInCone(Nz::Vector3f direction)
	{
		if (!IsConeScanReady())
			return {}; //< Scan not ready

		direction.Normalize();

		const Ndk::EntityHandle& spaceship = GetSpaceship();

		Ndk::World* world = spaceship->GetWorld();
		Nz::PhysWorld3D& physWorld = world->GetSystem<Ndk::PhysicsSystem3D>().GetWorld();

		constexpr float coneAngle = 90.f; //< Don't change this before looking at coneRadius definition
		constexpr float coneHeight = 10'000;
		constexpr float coneRadius = coneHeight; //< height / std::tan(Nz::ToRadian(angle/2))

		// Compute our scan cone AABB
		Nz::Vector3f coneOrigin = spaceship->GetComponent<Ndk::NodeComponent>().GetPosition();
		Nz::Vector3f coneBase = coneOrigin + direction * coneHeight;

		Nz::Vector3f lExtend = Nz::Vector3f::Left() * coneRadius;
		Nz::Vector3f uExtend = Nz::Vector3f::Up() * coneRadius;

		// And we add the four extremities of our pyramid
		Nz::Boxf coneLocalAABB(coneOrigin.x, coneOrigin.y, coneOrigin.z, 0.f, 0.f, 0.f);
		coneLocalAABB.ExtendTo(coneBase + lExtend + uExtend);
		coneLocalAABB.ExtendTo(coneBase + lExtend - uExtend);
		coneLocalAABB.ExtendTo(coneBase - lExtend + uExtend);
		coneLocalAABB.ExtendTo(coneBase - lExtend - uExtend);

		Nz::OrientedBoxf coneOBB(coneLocalAABB);
		coneOBB.Update(Nz::Matrix4f::Rotate(Nz::Quaternionf::RotationBetween(Nz::Vector3f::Forward(), direction)));

		Nz::Boxf coneGlobalAABB(coneOBB(0), coneOBB(1));
		for (unsigned int i = 2; i < 8; ++i)
			coneGlobalAABB.ExtendTo(coneOBB(i));

		ConeScanResults scanResults;

		physWorld.ForEachBodyInAABB(coneGlobalAABB, [this, &scanResults, world](const Nz::RigidBody3D& body)
		{
			//TODO: Check if entity is in the cone?
			Ndk::EntityId entityId = static_cast<Ndk::EntityId>(reinterpret_cast<std::ptrdiff_t>(body.GetUserdata()));
			Nz::Vector3f entityPos = body.GetPosition();

			const Ndk::EntityHandle& entity = world->GetEntity(entityId);
			if (entity == GetSpaceship())
				return true;

			const std::string& type = entity->GetComponent<SynchronizedComponent>().GetType();
			if (type == "projectile")
				return true;

			ConeScanResult result;
			result.id = entityId;
			result.pos = entityPos;
			result.type = type;

			scanResults.emplace_back(std::move(result));

			m_visibleEntities.Insert(world->GetEntity(entityId));
			return true;
		});

		m_lastConeScanTime = Nz::GetElapsedMilliseconds();

		return scanResults;
	}

	std::optional<RadarModule::ScanResult> RadarModule::ScanTarget(Ndk::EntityId targetId)
	{
		if (!IsTargetScanReady())
			return {}; //< Scan not ready

		if (!m_visibleEntities.Has(targetId))
			return {}; //< Entity not in visible range

		const Ndk::EntityHandle& entity = GetSpaceship()->GetWorld()->GetEntity(targetId);

		auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
		auto& syncComponent = entity->GetComponent<SynchronizedComponent>();

		ScanResult scanResult;
		scanResult.position = nodeComponent.GetPosition();
		scanResult.rotation = nodeComponent.GetRotation();
		scanResult.name = syncComponent.GetName();
		scanResult.type = syncComponent.GetType();

		if (entity->HasComponent<Ndk::PhysicsComponent3D>())
		{
			auto& physComponent = entity->GetComponent<Ndk::PhysicsComponent3D>();
			scanResult.angularVelocity = physComponent.GetAngularVelocity();
			scanResult.linearVelocity = physComponent.GetLinearVelocity();
		}
		else
		{
			scanResult.angularVelocity = Nz::Vector3f::Zero();
			scanResult.linearVelocity = Nz::Vector3f::Zero();
		}

		m_lastTargetScanTime = Nz::GetElapsedMilliseconds();

		return scanResult;
	}

	void RadarModule::Register(Nz::LuaState& lua)
	{
		if (!s_binding)
		{
			s_binding.emplace("Radar");

			s_binding->BindMethod("IsConeScanReady", &RadarModule::IsConeScanReady);
			s_binding->BindMethod("IsTargetScanReady", &RadarModule::IsTargetScanReady);

			// Workaround for value reply bug
			//s_binding->BindMethod("ScanInCone", &RadarModule::ScanInCone);
			s_binding->BindMethod("ScanInCone", [](Nz::LuaState& state, RadarModule* radar, std::size_t /*argCount*/)
			{
				int argIndex = 2;
				decltype(auto) result = radar->ScanInCone(state.Check<Nz::Vector3f>(&argIndex));
				if (result.has_value())
				{
					state.PushTable(result.value().size());
					std::size_t index = 0;
					for (auto& val : result.value())
					{
						state.PushInteger(index++);
						state.PushTable(2, 0);
							state.PushField("id", val.id);
							state.PushField("position", val.pos);
							state.PushField("type", val.type);

						state.SetTable();
					}
				}
				else
					state.PushNil();

				return 1;
			});

			s_binding->BindMethod("ScanTarget", [](Nz::LuaState& state, RadarModule* radar, std::size_t /*argCount*/)
			{
				int argIndex = 2;
				decltype(auto) result = radar->ScanTarget(state.Check<Ndk::EntityId>(&argIndex));
				if (result.has_value())
					return LuaImplReplyVal(state, result.value(), Nz::TypeTag<decltype(result.value())>());
				else
				{
					state.PushNil();
					return 1;
				}
			});


			//s_binding->BindMethod("ScanTarget", &RadarModule::ScanTarget);
		}

		s_binding->Register(lua);

		lua.PushField("Radar", this);
	}

	std::optional<Nz::LuaClass<RadarModuleHandle>> RadarModule::s_binding;
}
