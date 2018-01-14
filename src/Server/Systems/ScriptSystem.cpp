// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Systems/ScriptSystem.hpp>
#include <Server/Components/ScriptComponent.hpp>

namespace ewn
{
	ScriptSystem::ScriptSystem(ServerApplication* app) :
	m_app(app)
	{
		Requires<ScriptComponent>();

		SetMaximumUpdateRate(30.f);
	}

	void ScriptSystem::OnUpdate(float elapsedTime)
	{
		for (const Ndk::EntityHandle& entity : GetEntities())
		{
			ScriptComponent& script = entity->GetComponent<ScriptComponent>();

			script.Run(m_app, elapsedTime);
		}
	}

	Ndk::SystemIndex ScriptSystem::systemIndex;
}
