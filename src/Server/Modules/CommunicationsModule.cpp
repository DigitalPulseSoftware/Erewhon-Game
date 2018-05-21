// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Modules/CommunicationsModule.hpp>
#include <Nazara/Core/MemoryHelper.hpp>
#include <Nazara/Physics3D/PhysWorld3D.hpp>
#include <NDK/LuaAPI.hpp>
#include <NDK/World.hpp>
#include <NDK/Systems/PhysicsSystem3D.hpp>
#include <Server/Scripting/LuaMathTypes.hpp>
#include <Server/Components/CommunicationComponent.hpp>
#include <cmath>

namespace ewn
{
	void CommunicationsModule::Initialize(Ndk::Entity* spaceship)
	{
		if (!spaceship->HasComponent<CommunicationComponent>())
			spaceship->AddComponent<CommunicationComponent>();

		m_onReceivedMessageSlot.Connect(spaceship->GetComponent<CommunicationComponent>().OnReceivedMessage, this, &CommunicationsModule::OnReceivedMessage);
	}

	void CommunicationsModule::BroadcastCone(const Nz::Vector3f& direction, float distance, const std::string& message)
	{
		const Ndk::EntityHandle& spaceship = GetSpaceship();
		auto& spaceshipNode = spaceship->GetComponent<Ndk::NodeComponent>();

		float coneBaseRadius = std::tan(Nz::DegreeToRadian(30.f)) * distance;

		Nz::Vector3f position = spaceshipNode.GetPosition();
		Nz::Boxf detectionBox = Nz::Boxf(spaceshipNode.GetPosition() + spaceshipNode.GetLeft() * coneBaseRadius + spaceshipNode.GetUp() * coneBaseRadius, spaceshipNode.GetPosition() + spaceshipNode.GetForward() * distance + spaceshipNode.GetRight() * coneBaseRadius + spaceshipNode.GetDown() * coneBaseRadius);

		Ndk::World* world = spaceship->GetWorld();
		Nz::PhysWorld3D& physWorld = world->GetSystem<Ndk::PhysicsSystem3D>().GetWorld();
		physWorld.ForEachBodyInAABB(detectionBox, [&](Nz::RigidBody3D& body)
		{
			Nz::Vector3f bodyPosition = body.GetPosition();
			//TODO: Check for cone
			if (/*cone check*/ true)
			{
				Ndk::EntityId bodyId = static_cast<Ndk::EntityId>(reinterpret_cast<std::ptrdiff_t>(body.GetUserdata()));
				if (bodyId != spaceship->GetId())
				{
					const Ndk::EntityHandle& bodyEntity = world->GetEntity(bodyId);
					if (bodyEntity->HasComponent<CommunicationComponent>())
						bodyEntity->GetComponent<CommunicationComponent>().SendMessage(spaceship, message);
				}
			}

			return true;
		});
	}

	void CommunicationsModule::BroadcastSphere(float distance, const std::string& message)
	{
		const Ndk::EntityHandle& spaceship = GetSpaceship();
		auto& spaceshipNode = spaceship->GetComponent<Ndk::NodeComponent>();

		Nz::Vector3f position = spaceshipNode.GetPosition();
		Nz::Boxf detectionBox = Nz::Boxf(position - Nz::Vector3f(distance), position + Nz::Vector3f(distance));

		Ndk::World* world = spaceship->GetWorld();
		Nz::PhysWorld3D& physWorld = world->GetSystem<Ndk::PhysicsSystem3D>().GetWorld();
		float maxSquaredRadius = distance * distance;
		physWorld.ForEachBodyInAABB(detectionBox, [&](Nz::RigidBody3D& body)
		{
			Nz::Vector3f bodyPosition = body.GetPosition();
			if (bodyPosition.SquaredDistance(position) < maxSquaredRadius)
			{
				Ndk::EntityId bodyId = static_cast<Ndk::EntityId>(reinterpret_cast<std::ptrdiff_t>(body.GetUserdata()));
				if (bodyId != spaceship->GetId())
				{
					const Ndk::EntityHandle& bodyEntity = world->GetEntity(bodyId);
					if (bodyEntity->HasComponent<CommunicationComponent>())
						bodyEntity->GetComponent<CommunicationComponent>().SendMessage(spaceship, message);
				}
			}

			return true;
		});
	}

	void CommunicationsModule::Register(Nz::LuaState& lua)
	{
		if (!s_binding)
		{
			s_binding.emplace("Communications");

			s_binding->BindMethod("BroadcastCone",   &CommunicationsModule::BroadcastCone);
			s_binding->BindMethod("BroadcastSphere", &CommunicationsModule::BroadcastSphere);
		}

		s_binding->Register(lua);

		lua.PushField("Communications", this);
	}

	void CommunicationsModule::Run(float elapsedTime)
	{
		m_callbackCounter += elapsedTime;
		if (m_callbackCounter >= 1.f)
		{
			m_callbackCounter -= 1.f;

			if (!m_pendingMessages.empty())
			{
				const Ndk::EntityHandle& spaceship = GetSpaceship();
				auto& spaceshipNode = spaceship->GetComponent<Ndk::NodeComponent>();

				PushCallback("OnCommunicationReceivedMessages", [messages = m_pendingMessages, position = spaceshipNode.GetPosition()](Nz::LuaState& state)
				{
					state.PushTable(messages.size());

					std::size_t index = 1;
					for (const auto& messageData : messages)
					{
						float distance;
						Nz::Vector3f direction = messageData.position - position;
						direction.Normalize(&distance);

						state.Push(index++);
						state.PushTable(0, 3);
						{
							state.PushField("data", messageData.message);
							state.PushField("direction", LuaVec3(direction));
							state.PushField("distance", distance);
						}

						state.SetTable();
					}

					return 1;
				}, false);

				m_pendingMessages.clear();
			}
		}
	}

	void CommunicationsModule::OnReceivedMessage(CommunicationComponent*, const Ndk::EntityHandle& emitter, const std::string& message)
	{
		Nz::Vector3f emitterPos = emitter->GetComponent<Ndk::NodeComponent>().GetPosition();

		PendingMessage& newMessage = m_pendingMessages.emplace_back();
		newMessage.message = message;
		newMessage.position = emitterPos;
	}

	std::optional<Nz::LuaClass<CommunicationsModuleHandle>> CommunicationsModule::s_binding;
}
