// Copyright (C) 2018 Jérôme Leclercq
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
#include <Server/Components/SignatureComponent.hpp>
#include <Server/Components/SynchronizedComponent.hpp>
#include <Server/Scripting/LuaTypes.hpp>
#include <iostream>

namespace ewn
{
	std::optional<RadarModule::TargetInfo> RadarModule::GetTargetInfo(Nz::Int64 signature)
	{
		const Ndk::EntityHandle& target = FindEntityBySignature(signature);
		if (!target)
			return {};

		if (!m_entitiesInRadius.Has(target))
		{
			//TODO: Log?
			m_signatureToEntity.erase(signature);
			return {};
		}

		const Ndk::EntityHandle& spaceship = GetSpaceship();

		auto& targetNode = target->GetComponent<Ndk::NodeComponent>();

		TargetInfo targetInfo;

		float distance;
		Nz::Vector3f direction = targetNode.GetPosition() - spaceship->GetComponent<Ndk::NodeComponent>().GetPosition();
		direction.Normalize(&distance);

		targetInfo.direction = direction;
		targetInfo.distance = distance;
		targetInfo.rotation = targetNode.GetRotation();

		if (target->HasComponent<Ndk::PhysicsComponent3D>())
		{
			auto& physComponent = target->GetComponent<Ndk::PhysicsComponent3D>();
			targetInfo.angularVelocity = physComponent.GetAngularVelocity();
			targetInfo.linearVelocity = physComponent.GetLinearVelocity();
		}
		else
		{
			targetInfo.angularVelocity = Nz::Vector3f::Zero();
			targetInfo.linearVelocity = Nz::Vector3f::Zero();
		}

		if (target->HasComponent<SignatureComponent>())
		{
			auto& targetSignature = target->GetComponent<SignatureComponent>();
			targetInfo.emSignature = targetSignature.GetEmSignature();
			targetInfo.signature = targetSignature.GetSignature();
			targetInfo.size = targetSignature.GetSize();
			targetInfo.volume = targetSignature.GetVolume();
		}
		else
		{
			targetInfo.emSignature = 0.0;
			targetInfo.signature = target->GetId(); //< Meh
			targetInfo.size = 0.f;
			targetInfo.volume = 0.f;
		}

		return targetInfo;
	}

	std::vector<RadarModule::RangeInfo> RadarModule::Scan()
	{
		const Ndk::EntityHandle& spaceship = GetSpaceship();
		Nz::Vector3f spaceshipPosition = spaceship->GetComponent<Ndk::NodeComponent>().GetPosition();

		std::vector<RadarModule::RangeInfo> targetInfos;

		targetInfos.reserve(m_entitiesInRadius.size());
		for (const Ndk::EntityHandle& target : m_entitiesInRadius)
		{
			auto& info = targetInfos.emplace_back();

			auto& targetNode = target->GetComponent<Ndk::NodeComponent>();

			float distance;
			Nz::Vector3f direction = targetNode.GetPosition() - spaceshipPosition;
			direction.Normalize(&distance);

			info.direction = direction;
			info.distance = distance;

			if (target->HasComponent<SignatureComponent>())
			{
				auto& targetSignature = target->GetComponent<SignatureComponent>();
				info.signature = targetSignature.GetSignature();
				info.emSignature = targetSignature.GetEmSignature();
				info.size = targetSignature.GetSize();
			}
			else
			{
				info.signature = target->GetId(); //< Meeeeh

				info.emSignature = 0.0;
				info.size = 0.f;
			}
		}

		return targetInfos;
	}

	void RadarModule::Register(Nz::LuaState& lua)
	{
		if (!s_binding)
		{
			s_binding.emplace("Radar");

			s_binding->BindMethod("EnablePassiveScan", &RadarModule::EnablePassiveScan);
			s_binding->BindMethod("GetTargetInfo", &RadarModule::GetTargetInfo);
			s_binding->BindMethod("IsPassiveScanEnabled", &RadarModule::IsPassiveScanEnabled);
			s_binding->BindMethod("Scan", &RadarModule::Scan);

			// Workaround for value reply bug
			s_binding->BindMethod("GetTargetInfo", [](Nz::LuaState& state, RadarModule* radar, std::size_t /*argCount*/)
			{
				int argIndex = 2;
				decltype(auto) result = radar->GetTargetInfo(state.Check<Nz::Int64>(&argIndex));
				if (result.has_value())
					state.Push(*result);
				else
					state.PushNil();

				return 1;
			});

			s_binding->BindMethod("Scan", [](Nz::LuaState& state, RadarModule* radar, std::size_t /*argCount*/)
			{
				int argIndex = 2;
				std::vector<RangeInfo> result = radar->Scan();

				state.PushTable(result.size(), 0);

				std::size_t index = 1;
				for (const RangeInfo& info : result)
				{
					state.Push(index++); // key
					state.Push(info); // value

					state.SetTable();
				}

				return 1;
			});
		}

		s_binding->Register(lua);

		lua.PushField("Radar", this);
	}

	void RadarModule::Run(float /*elapsedTime*/)
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

		Nz::Vector3f radarCenter = spaceshipNode.GetPosition();

		float maxDetectionRadiusSq = m_detectionRadius * m_detectionRadius;
		for (const Ndk::EntityHandle& target : m_entitiesInRadius)
		{
			auto& targetNode = target->GetComponent<Ndk::NodeComponent>();
			if (targetNode.GetPosition().SquaredDistance(radarCenter) > maxDetectionRadiusSq)
			{
				m_entitiesInRadius.Remove(target);

				if (target->HasComponent<SignatureComponent>())
				{
					const SignatureComponent& component = target->GetComponent<SignatureComponent>();

					m_signatureToEntity.erase(component.GetSignature());
				}
			}
		}
	}

	void RadarModule::PerformScan()
	{
		const Ndk::EntityHandle& spaceship = GetSpaceship();
		auto& spaceshipNode = spaceship->GetComponent<Ndk::NodeComponent>();

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

					m_entitiesInRadius.Insert(bodyEntity);

					Nz::Int64 signature = bodyEntity->GetId(); //< Meh
					float radius = -1.f;
					double emSignature = 0.0;
					if (bodyEntity->HasComponent<SignatureComponent>())
					{
						const SignatureComponent& component = bodyEntity->GetComponent<SignatureComponent>();
						emSignature = component.GetEmSignature();
						signature = component.GetSignature();
						radius = component.GetSize();

						m_signatureToEntity.insert_or_assign(signature, bodyEntity);
					}

					float distance;
					Nz::Vector3f direction = bodyPosition - position;
					direction.Normalize(&distance);

					PushCallback("OnRadarNewObjectInRange", [signature, emSignature, radius, direction, distance](Nz::LuaState& state)
					{
						state.Push(signature);
						state.Push(emSignature);
						state.Push(radius);
						state.Push(LuaVec3(direction));
						state.Push(distance);

						return 5;
					},
					false);
				}
			}

			return true;
		});
	}

	std::optional<Nz::LuaClass<RadarModuleHandle>> RadarModule::s_binding;
}
