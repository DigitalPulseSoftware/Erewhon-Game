// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Systems/LifeTimeSystem.hpp>
#include <Server/Components/LifeTimeComponent.hpp>

namespace ewn
{
	LifeTimeSystem::LifeTimeSystem()
	{
		Requires<LifeTimeComponent>();
	}

	void LifeTimeSystem::OnUpdate(float elapsedTime)
	{
		for (const Ndk::EntityHandle& entity : GetEntities())
		{
			LifeTimeComponent& lifeTime = entity->GetComponent<LifeTimeComponent>();

			if (lifeTime.DecreaseDuration(elapsedTime))
				entity->Kill();
		}
	}

	Ndk::SystemIndex LifeTimeSystem::systemIndex;
}
