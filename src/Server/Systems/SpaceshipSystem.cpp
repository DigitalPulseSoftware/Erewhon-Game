// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Systems/SpaceshipSystem.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <Server/Components/PlayerControlledComponent.hpp>

namespace ewn
{
	SpaceshipSystem::SpaceshipSystem()
	{
		Requires<Ndk::NodeComponent, PlayerControlledComponent>();
		SetMaximumUpdateRate(60.f);
	}

	void SpaceshipSystem::OnUpdate(float elapsedTime)
	{
		for (const Ndk::EntityHandle& spaceship : GetEntities())
		{
			auto& spaceshipNode = spaceship->GetComponent<Ndk::NodeComponent>();
			auto& spaceshipControl = spaceship->GetComponent<PlayerControlledComponent>();

			const Nz::Vector3f& spaceshipMovement = spaceshipControl.GetDirection();
			const Nz::Vector3f& spaceshipRotation = spaceshipControl.GetRotation();

			spaceshipNode.Move(elapsedTime * (spaceshipMovement.x * Nz::Vector3f::Forward() + spaceshipMovement.y * Nz::Vector3f::Left() + spaceshipMovement.z * Nz::Vector3f::Up()));
			spaceshipNode.Rotate(Nz::EulerAnglesf(spaceshipRotation.x * elapsedTime, spaceshipRotation.y * elapsedTime, spaceshipRotation.z * elapsedTime));
		}
	}

	Ndk::SystemIndex SpaceshipSystem::systemIndex;
}
