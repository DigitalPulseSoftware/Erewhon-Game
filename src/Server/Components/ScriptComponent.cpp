// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/ScriptComponent.hpp>
#include <Nazara/Core/CallOnExit.hpp>
#include <Nazara/Math/Vector3.hpp>
#include <NDK/LuaAPI.hpp>
#include <Server/ServerApplication.hpp>
#include <Server/Components/InputComponent.hpp>
#include <Server/Modules/WeaponModule.hpp>
#include <iostream>

namespace ewn
{
	ScriptComponent::ScriptComponent(std::string scriptName) :
	m_scriptName(std::move(scriptName))
	{
	}

	ScriptComponent::ScriptComponent(const ScriptComponent& component) :
	ScriptComponent(component.m_scriptName)
	{
	}

	void ScriptComponent::Run(ServerApplication* app, float elapsedTime)
	{
		if (m_instance.GetGlobal("Spaceship") == Nz::LuaType_Table)
		{
			if (m_instance.GetField("OnTick") == Nz::LuaType_Function)
			{
				m_instance.PushValue(-2); // Spaceship
				m_instance.Push(elapsedTime);
				if (!m_instance.Call(2, 0))
					std::cerr << "Spaceship crashed: " << m_instance.GetLastError() << std::endl;
			}
			else
				m_instance.Pop();
		}
		m_instance.Pop();

		if (m_instance.GetGlobal("Spaceship") == Nz::LuaType_Table)
		{
			if (m_instance.GetField("OnUpdateInput") == Nz::LuaType_Function)
			{
				m_instance.PushValue(-2); // Spaceship
				m_instance.Push(elapsedTime);
				if (!m_instance.Call(2, 6))
					std::cerr << "Spaceship crashed (OnUpdateInput): " << m_instance.GetLastError() << std::endl;

				// Use some RRID
				Nz::CallOnExit resetLuaStack([&]()
				{
					m_instance.Pop(m_instance.GetStackTop());
				});

				Nz::Vector3f movement;
				Nz::Vector3f rotation;

				try
				{
					int index = 2;
					movement = m_instance.Check<Nz::Vector3f>(&index);
					rotation = m_instance.Check<Nz::Vector3f>(&index);
				}
				catch (const std::exception&)
				{
					std::cerr << "OnUpdateInput failed: returned values are invalid:\n";
					std::cerr << m_instance.DumpStack() << std::endl;
					return;
				}

				auto& inputComponent = m_entity->GetComponent<InputComponent>();
				inputComponent.PushInput(app->GetAppTime(), movement, rotation);
			}
			else
				m_instance.Pop();
		}
		m_instance.Pop();
	}

	void ScriptComponent::OnAttached()
	{
		m_core.emplace(m_entity);
		m_core->AddModule(std::make_unique<WeaponModule>(m_entity));

		m_instance.PushTable();
		{
			m_core->Register(m_instance);
		}
		m_instance.SetGlobal("Spaceship");

		if (!m_instance.ExecuteFromFile(m_scriptName))
		{
			std::cerr << m_instance.GetLastError() << std::endl;
			assert(false);
		}
	}

	void ScriptComponent::OnDetached()
	{
		m_core.reset();
	}

	Ndk::ComponentIndex ScriptComponent::componentIndex;
}
