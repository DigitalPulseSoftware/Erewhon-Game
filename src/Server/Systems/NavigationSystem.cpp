// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Systems/NavigationSystem.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent3D.hpp>
#include <Server/Components/InputComponent.hpp>
#include <Server/Components/NavigationComponent.hpp>
#include <Server/Components/PlayerControlledComponent.hpp>
#include <Server/ServerApplication.hpp>

namespace ewn
{
	NavigationSystem::NavigationSystem() :
	m_timeAccumulator(0.f)
	{
		Requires<InputComponent, NavigationComponent, Ndk::NodeComponent, Ndk::PhysicsComponent3D>();
		Excludes<PlayerControlledComponent>();
		SetFixedUpdateRate(30.f);

		m_appTime = ServerApplication::GetAppTime();
	}

	void NavigationSystem::OnUpdate(float elapsedTime)
	{
		// Handle app time via elapsed time, to handle fixed update rate
		m_timeAccumulator += elapsedTime;
		float elapsedMs = std::floor(m_timeAccumulator * 1'000);
		m_timeAccumulator -= elapsedMs / 1'000;

		m_appTime += static_cast<Nz::UInt64>(elapsedMs);

		for (const Ndk::EntityHandle& entity : GetEntities())
		{
			Ndk::NodeComponent& entityNode = entity->GetComponent<Ndk::NodeComponent>();
			Ndk::PhysicsComponent3D& entityPhys = entity->GetComponent<Ndk::PhysicsComponent3D>();
			InputComponent& entityInput = entity->GetComponent<InputComponent>();
			NavigationComponent& entityNavigation = entity->GetComponent<NavigationComponent>();

			auto [thrust, rotation, isCloseEnough] = entityNavigation.Run(m_appTime, elapsedTime, entityNode.GetPosition(), entityNode.GetRotation(), entityPhys.GetLinearVelocity(), entityPhys.GetAngularVelocity());

			entityInput.PushInput(m_appTime, thrust, rotation);
		}
	}

	Ndk::SystemIndex NavigationSystem::systemIndex;
}
