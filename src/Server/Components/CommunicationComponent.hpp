// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_COMMUNICATIONCOMPONENT_HPP
#define EREWHON_SERVER_COMMUNICATIONCOMPONENT_HPP

#include <Nazara/Core/Signal.hpp>
#include <NDK/Component.hpp>

namespace ewn
{
	class CommunicationComponent : public Ndk::Component<CommunicationComponent>
	{
		public:
			CommunicationComponent() = default;
			inline CommunicationComponent(const CommunicationComponent& commComponent);

			inline void SendMessage(const Ndk::EntityHandle& from, const std::string& message);

			static Ndk::ComponentIndex componentIndex;

			NazaraSignal(OnReceivedMessage, CommunicationComponent* /*communication*/, const Ndk::EntityHandle& /*messageEmitter*/, const std::string& /*message*/);
	};
}

#include <Server/Components/CommunicationComponent.inl>

#endif // EREWHON_SERVER_COMMUNICATIONCOMPONENT_HPP
