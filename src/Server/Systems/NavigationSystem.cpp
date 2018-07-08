// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
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
	NavigationSystem::NavigationSystem(ServerApplication* app) :
	m_app(app)
	{
		Requires<InputComponent, NavigationComponent, Ndk::NodeComponent, Ndk::PhysicsComponent3D>();
		Excludes<PlayerControlledComponent>();
		SetFixedUpdateRate(30.f);
	}

	void NavigationSystem::OnUpdate(float elapsedTime)
	{
		Nz::UInt64 appTime = m_app->GetAppTime();

		for (const Ndk::EntityHandle& entity : GetEntities())
		{
			Ndk::NodeComponent& entityNode = entity->GetComponent<Ndk::NodeComponent>();
			Ndk::PhysicsComponent3D& entityPhys = entity->GetComponent<Ndk::PhysicsComponent3D>();
			InputComponent& entityInput = entity->GetComponent<InputComponent>();
			NavigationComponent& entityNavigation = entity->GetComponent<NavigationComponent>();

			auto [thrust, rotation, isCloseEnough] = entityNavigation.Run(appTime, elapsedTime, entityNode.GetPosition(), entityNode.GetRotation(), entityPhys.GetLinearVelocity(), entityPhys.GetAngularVelocity());

			entityInput.PushInput(appTime, thrust, rotation);
		}
	}

	Ndk::SystemIndex NavigationSystem::systemIndex;
}
