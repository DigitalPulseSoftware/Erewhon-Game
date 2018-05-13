// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/SignatureComponent.hpp>

namespace ewn
{
	inline SignatureComponent::SignatureComponent(Nz::Int64 signature, float size, float volume) :
	m_signature(signature),
	m_size(size),
	m_volume(volume)
	{
	}

	inline Nz::Int64 SignatureComponent::GetSignature() const
	{
		return m_signature;
	}

	inline float SignatureComponent::GetSize() const
	{
		return m_size;
	}

	inline float SignatureComponent::GetVolume() const
	{
		return m_volume;
	}
}
