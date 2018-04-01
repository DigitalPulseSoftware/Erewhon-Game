// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Systems/ScriptSystem.hpp>
#include <Server/Arena.hpp>
#include <Server/Components/OwnerComponent.hpp>
#include <Server/Components/ScriptComponent.hpp>
#include <Server/Components/SynchronizedComponent.hpp>

namespace ewn
{
	ScriptSystem::ScriptSystem(ServerApplication* app, Arena* arena) :
	m_arena(arena),
	m_app(app)
	{
		Requires<ScriptComponent>();

		SetMaximumUpdateRate(100.f);
	}

	void ScriptSystem::OnUpdate(float elapsedTime)
	{
		for (const Ndk::EntityHandle& entity : GetEntities())
		{
			ScriptComponent& script = entity->GetComponent<ScriptComponent>();

			Nz::String lastError;
			if (!script.Run(m_app, elapsedTime, &lastError))
				script.SendMessage(BotMessageType::Error, lastError);
		}
	}

	Ndk::SystemIndex ScriptSystem::systemIndex;
}
