// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Systems/SpaceshipSystem.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <Server/Components/PlayerControlledComponent.hpp>
#include <iostream>

namespace ewn
{
	SpaceshipSystem::SpaceshipSystem()
	{
		Requires<Ndk::NodeComponent, PlayerControlledComponent>();
		SetMaximumUpdateRate(60.f);
	}

	void SpaceshipSystem::OnUpdate(float /*elapsedTime*/)
	{
		for (const Ndk::EntityHandle& spaceship : GetEntities())
		{
			auto& spaceshipNode = spaceship->GetComponent<Ndk::NodeComponent>();
			auto& spaceshipControl = spaceship->GetComponent<PlayerControlledComponent>();

			Nz::UInt64 lastInput = spaceshipControl.GetLastInputTime();
			spaceshipControl.ProcessInputs([&] (Nz::UInt64 time, const Nz::Vector3f& movement, const Nz::Vector3f& rotation)
			{
				float elapsedTime = (time - lastInput) / 1000.f;
				lastInput = time;

				Nz::Vector3f totalMovement = elapsedTime * (movement.x * Nz::Vector3f::Forward() + movement.y * Nz::Vector3f::Left() + movement.z * Nz::Vector3f::Up());
				Nz::EulerAnglesf totalRotation = Nz::EulerAnglesf(rotation.x * elapsedTime, rotation.y * elapsedTime, rotation.z * elapsedTime);

				spaceshipNode.Move(totalMovement);
				spaceshipNode.Rotate(totalRotation);

				/*std::cout << "At " << time << ": Move by " << totalMovement << " (final pos: " << spaceshipNode.GetPosition() << ")\n";
				std::cout << "   " << time << ": Rotate by " << totalRotation << " (final pos: " << spaceshipNode.GetRotation().ToEulerAngles() << ')' << std::endl;*/
			});
		}
	}

	Ndk::SystemIndex SpaceshipSystem::systemIndex;
}
