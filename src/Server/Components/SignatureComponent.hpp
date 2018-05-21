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
			inline SignatureComponent(Nz::Int64 signature, double emSignature, double size, double volume);

			inline double GetEmSignature() const;
			inline Nz::Int64 GetSignature() const;
			inline double GetSize() const;
			inline double GetVolume() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			Nz::Int64 m_signature;
			double m_emSignature;
			double m_size;
			double m_volume;
	};
}

#include <Server/Components/SignatureComponent.inl>

#endif // EREWHON_SERVER_SIGNATURECOMPONENT_HPP
