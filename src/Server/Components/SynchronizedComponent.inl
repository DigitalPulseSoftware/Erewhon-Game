// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/SynchronizedComponent.hpp>

namespace ewn
{
	inline SynchronizedComponent::SynchronizedComponent(std::string type, std::string nameTemp, bool movable) :
	m_name(std::move(nameTemp)),
	m_type(std::move(type)),
	m_movable(movable)
	{
	}

	inline const std::string & SynchronizedComponent::GetName() const
	{
		return m_name;
	}

	inline const std::string& SynchronizedComponent::GetType() const
	{
		return m_type;
	}

	inline bool SynchronizedComponent::IsMovable() const
	{
		return m_movable;
	}
}
