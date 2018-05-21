// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/SignatureComponent.hpp>

namespace ewn
{
	inline SignatureComponent::SignatureComponent(Nz::Int64 signature, double emSignature, double size, double volume) :
	m_signature(signature),
	m_emSignature(emSignature),
	m_size(size),
	m_volume(volume)
	{
	}

	inline double SignatureComponent::GetEmSignature() const
	{
		return m_emSignature;
	}

	inline Nz::Int64 SignatureComponent::GetSignature() const
	{
		return m_signature;
	}

	inline double SignatureComponent::GetSize() const
	{
		return m_size;
	}

	inline double SignatureComponent::GetVolume() const
	{
		return m_volume;
	}
}
