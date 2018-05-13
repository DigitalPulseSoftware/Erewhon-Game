// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_SIGNATURECOMPONENT_HPP
#define EREWHON_SERVER_SIGNATURECOMPONENT_HPP

#include <NDK/Component.hpp>

namespace ewn
{
	class SignatureComponent : public Ndk::Component<SignatureComponent>
	{
		public:
			inline SignatureComponent(Nz::Int64 signature, float size, float volume);

			inline Nz::Int64 GetSignature() const;
			inline float GetSize() const;
			inline float GetVolume() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			Nz::Int64 m_signature;
			float m_size;
			float m_volume;
	};
}

#include <Server/Components/SignatureComponent.inl>

#endif // EREWHON_SERVER_SIGNATURECOMPONENT_HPP
