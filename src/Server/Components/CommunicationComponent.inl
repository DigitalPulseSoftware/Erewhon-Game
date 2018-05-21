// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/CommunicationComponent.hpp>

namespace ewn
{
	inline CommunicationComponent::CommunicationComponent(const CommunicationComponent& commComponent) :
	Component(commComponent)
	{
	}

	inline void CommunicationComponent::SendMessage(const Ndk::EntityHandle& from, const std::string& message)
	{
		OnReceivedMessage(this, from, message);
	}
}
