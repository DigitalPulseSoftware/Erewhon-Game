// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/ScriptComponent.hpp>
#include <Nazara/Core/CallOnExit.hpp>
#include <Nazara/Core/Clock.hpp>
#include <Nazara/Lua/LuaInstance.hpp>
#include <Nazara/Math/Vector3.hpp>
#include <NDK/LuaAPI.hpp>
#include <Server/ModuleStore.hpp>
#include <Server/ServerApplication.hpp>
#include <Server/Components/InputComponent.hpp>
#include <Server/Components/OwnerComponent.hpp>
#include <Server/Modules/EngineModule.hpp>
#include <Server/Modules/NavigationModule.hpp>
#include <Server/Modules/RadarModule.hpp>
#include <Server/Modules/WeaponModule.hpp>
#include <iostream>

namespace ewn
{
	ScriptComponent::ScriptComponent() :
	m_lastMessageTime(0),
	m_tickCounter(0.f)
	{
		m_instance.SetMemoryLimit(1'000'000);
		m_instance.SetTimeLimit(50);

		m_instance.LoadLibraries(Nz::LuaLib_Math | Nz::LuaLib_String | Nz::LuaLib_Table | Nz::LuaLib_Utf8);

		m_instance.PushNil();
		m_instance.SetGlobal("collectgarbage");

		m_instance.PushNil();
		m_instance.SetGlobal("dofile");

		m_instance.PushNil();
		m_instance.SetGlobal("loadfile");

		// TODO: Refactor

		m_instance.PushFunction([this](const Nz::LuaState& state) -> int
		{
			SendMessage(BotMessageType::Info, state.CheckString(1));
			return 0;
		});

		m_instance.PushValue(-1); //< Copy previous function to keep it on stack
		m_instance.SetGlobal("print");
		m_instance.SetGlobal("notice");

		m_instance.PushFunction([this](const Nz::LuaState& state) -> int
		{
			SendMessage(BotMessageType::Warning, state.CheckString(1));
			return 0;
		});
		m_instance.SetGlobal("warn");

		if (!m_instance.ExecuteFromFile("spacelib.lua"))
			assert(!"Failed to load spacelib.lua");
	}

	ScriptComponent::ScriptComponent(const ScriptComponent& component) :
	ScriptComponent()
	{
		if (component.HasValidScript())
		{
			Nz::String lastError;
			if (!Execute(component.m_script, &lastError))
				NazaraError("ScriptComponent copy failed: " + lastError);
		}
	}

	bool ScriptComponent::Execute(Nz::String script, Nz::String* lastError)
	{
		if (!m_instance.Execute(script))
		{
			if (lastError)
				*lastError = m_instance.GetLastError();

			return false;
		}

		m_script = std::move(script);
		return true;
	}

	bool ScriptComponent::Initialize(ServerApplication* app, const std::vector<std::size_t>& moduleIds)
	{
		m_core.emplace(m_entity);

		const ModuleStore& moduleStore = app->GetModuleStore();
		for (std::size_t moduleId : moduleIds)
		{
			auto modulePtr = moduleStore.BuildModule(moduleId, &m_core.value(), m_entity);
			if (!modulePtr)
				return false;

			m_core->AddModule(std::move(modulePtr));
		}

		m_instance.PushTable();
		{
			m_core->Register(m_instance);
		}
		m_instance.SetGlobal("Spaceship");

		m_core->PushCallback(0, "OnStart");

		return true;
	}

	bool ScriptComponent::Run(ServerApplication* app, float elapsedTime, Nz::String* lastError)
	{
		assert(m_core);

		if (!HasValidScript())
			return true;

		m_core->Run();

		std::string callbackName;
		SpaceshipCore::CallbackArgFunction argFunction;

		Nz::CallOnExit incrementTickCount([&]()
		{
			m_tickCounter += elapsedTime;
		});

		if (m_tickCounter >= 0.5f)
		{
			callbackName = "OnTick";
			argFunction = [](Nz::LuaState& state)
			{
				state.Push(0.5f);
				return 1;
			};

			m_tickCounter -= 0.5f;
		}
		else
		{
			auto callback = m_core->PopCallback();
			if (!callback)
				return true;

			callbackName = std::move(callback->first);
			argFunction = std::move(callback->second);
		}

		incrementTickCount.CallAndReset();

		if (m_instance.GetGlobal("Spaceship") == Nz::LuaType_Table)
		{
			if (m_instance.GetField(callbackName) == Nz::LuaType_Function)
			{
				m_instance.PushValue(-2); // Spaceship

				unsigned int argCount = 1;
				if (argFunction)
					argCount += argFunction(m_instance);

				if (!m_instance.Call(argCount, 0))
				{
					if (lastError)
						*lastError = m_instance.GetLastError();

					m_script = Nz::String();
					return false;
				}
			}
			else
				m_instance.Pop();
		}
		m_instance.Pop();

		return true;
	}

	void ScriptComponent::SendMessage(BotMessageType messageType, Nz::String message)
	{
		Nz::UInt64 now = Nz::GetElapsedMilliseconds();
		//if (messageType != BotMessageType::Error && now - m_lastMessageTime < 100)
		//	return;

		m_lastMessageTime = now;

		// TODO: Refactor this to prevent inter-component dependency
		if (m_entity->HasComponent<OwnerComponent>())
		{
			if (Player* owner = m_entity->GetComponent<OwnerComponent>().GetOwner())
			{
				constexpr std::size_t MaxMessageSize = 255;
				if (message.GetSize() > MaxMessageSize)
				{
					message.Resize(MaxMessageSize - 3, Nz::String::HandleUtf8);
					message += "...";
				}

				Packets::BotMessage messagePacket;
				messagePacket.messageType = messageType;
				messagePacket.errorMessage = message.ToStdString();

				owner->SendPacket(messagePacket);
			}
		}
	}

	void ScriptComponent::OnDetached()
	{
		m_core.reset();
	}

	Ndk::ComponentIndex ScriptComponent::componentIndex;
}
