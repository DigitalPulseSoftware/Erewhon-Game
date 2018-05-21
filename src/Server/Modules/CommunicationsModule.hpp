// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_COMMUNICATIONSMODULE_HPP
#define EREWHON_SERVER_COMMUNICATIONSMODULE_HPP

#include <Nazara/Core/HandledObject.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Lua/LuaClass.hpp>
#include <Nazara/Math/Vector3.hpp>
#include <Server/SpaceshipModule.hpp>
#include <Server/Components/CommunicationComponent.hpp>
#include <optional>
#include <vector>

namespace ewn
{
	class CommunicationsModule;

	using CommunicationsModuleHandle = Nz::ObjectHandle<CommunicationsModule>;

	class CommunicationsModule : public SpaceshipModule, public Nz::HandledObject<CommunicationsModule>
	{
		public:
			inline CommunicationsModule(SpaceshipCore* core, const Ndk::EntityHandle& spaceship);
			~CommunicationsModule() = default;

			void Initialize(Ndk::Entity* spaceship) override;

			void BroadcastCone(const Nz::Vector3f& direction, float distance, const std::string& message);
			void BroadcastSphere(float distance, const std::string& message);

			void Register(Nz::LuaState& lua) override;
			void Run(float elapsedTime) override;

		private:
			void OnReceivedMessage(CommunicationComponent* /*communication*/, const Ndk::EntityHandle& emitter, const std::string& message);

			NazaraSlot(CommunicationComponent, OnReceivedMessage, m_onReceivedMessageSlot);

			struct PendingMessage
			{
				Nz::Vector3f position;
				std::string message;
			};

			float m_callbackCounter;
			std::vector<PendingMessage> m_pendingMessages;

			static std::optional<Nz::LuaClass<CommunicationsModuleHandle>> s_binding;
	};
}

#include <Server/Modules/CommunicationsModule.inl>

#endif // EREWHON_SERVER_COMMUNICATIONSMODULE_HPP
