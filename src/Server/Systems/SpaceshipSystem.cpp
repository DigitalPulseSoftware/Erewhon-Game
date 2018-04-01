// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Systems/SpaceshipSystem.hpp>
#include <Nazara/Utility/Node.hpp>
#include <NDK/Components/PhysicsComponent3D.hpp>
#include <Server/Components/InputComponent.hpp>
#include <iostream>

namespace ewn
{
	SpaceshipSystem::SpaceshipSystem()
	{
		Requires<Ndk::PhysicsComponent3D, InputComponent>();
		SetMaximumUpdateRate(60.f);
	}

	void SpaceshipSystem::OnUpdate(float /*elapsedTime*/)
	{
		for (const Ndk::EntityHandle& spaceship : GetEntities())
		{
			auto& spaceshipPhysics = spaceship->GetComponent<Ndk::PhysicsComponent3D>();
			auto& spaceshipInput = spaceship->GetComponent<InputComponent>();

			Nz::UInt64 lastInput = spaceshipInput.GetLastInputTime();
			spaceshipInput.ProcessInputs([&] (Nz::UInt64 time, const Nz::Vector3f& movement, const Nz::Vector3f& rotation)
			{
				static constexpr float AngularMultiplier = 3000.f;
				static constexpr float ForceMultiplier = 15000.f;

				float inputElapsedTime = (lastInput != 0) ? (time - lastInput) / 1000.f : 0.f;

				Nz::Vector3f totalMovement = inputElapsedTime * movement;
				Nz::Vector3f totalRotation = inputElapsedTime * rotation;

				spaceshipPhysics.AddForce(ForceMultiplier * totalMovement, Nz::CoordSys_Local);
				spaceshipPhysics.AddTorque(AngularMultiplier * totalRotation, Nz::CoordSys_Global);

				/*std::cout << "At " << time << ": Move by " << totalMovement << " (final pos: " << spaceshipNode.GetPosition() << ")\n";
				std::cout << "   " << time << ": Rotate by " << totalRotation << " (final pos: " << spaceshipNode.GetRotation().ToEulerAngles() << ')' << std::endl;*/

				lastInput = time;
			});
		}
	}

	Ndk::SystemIndex SpaceshipSystem::systemIndex;
}
