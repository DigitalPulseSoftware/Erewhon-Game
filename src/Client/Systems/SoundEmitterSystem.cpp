// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/Systems/SoundEmitterSystem.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <Client/Components/SoundEmitterComponent.hpp>

namespace ewn
{
	SoundEmitterSystem::SoundEmitterSystem()
	{
		Requires<Ndk::NodeComponent, SoundEmitterComponent>();
	}

	void SoundEmitterSystem::OnEntityAdded(Ndk::Entity* entity)
	{
		Ndk::NodeComponent& entityNode = entity->GetComponent<Ndk::NodeComponent>();
		SoundEmitterComponent& entitySoundEmitter = entity->GetComponent<SoundEmitterComponent>();

		entitySoundEmitter.UpdateLastPosition(entityNode.GetPosition());
	}

	void SoundEmitterSystem::OnUpdate(float elapsedTime)
	{
		float invElapsedTime = 1.f / elapsedTime;

		for (const Ndk::EntityHandle& entity : GetEntities())
		{
			Ndk::NodeComponent& entityNode = entity->GetComponent<Ndk::NodeComponent>();
			SoundEmitterComponent& entitySoundEmitter = entity->GetComponent<SoundEmitterComponent>();

			// Update source position
			Nz::Vector3f currentPosition = entityNode.GetPosition();
			entitySoundEmitter.SetPosition(currentPosition);

			// Update source velocity
			Nz::Vector3f lastPosition = entitySoundEmitter.GetLastPosition();
			Nz::Vector3f velocity = (currentPosition - lastPosition) * invElapsedTime;

			entitySoundEmitter.SetVelocity(velocity);
			entitySoundEmitter.UpdateLastPosition(currentPosition);
		}
	}

	Ndk::SystemIndex SoundEmitterSystem::systemIndex;
}
