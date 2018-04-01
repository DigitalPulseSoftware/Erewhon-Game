// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_NETWORK_COMPRESSEDINTEGER_HPP
#define EREWHON_SHARED_NETWORK_COMPRESSEDINTEGER_HPP

#include <Nazara/Core/Algorithm.hpp>
#include <type_traits>

namespace ewn
{
	template<typename T>
	class CompressedSigned
	{
		static_assert(std::is_signed_v<T>);

		public:
			explicit CompressedSigned(T value = 0);
			~CompressedSigned() = default;

			operator T() const;

			CompressedSigned& operator=(T value);

		private:
			T m_value;
	};

	template<typename T>
	class CompressedUnsigned
	{
		static_assert(std::is_unsigned_v<T>);

		public:
			explicit CompressedUnsigned(T value = 0);
			~CompressedUnsigned() = default;

			operator T() const;

			CompressedUnsigned& operator=(T value);

		private:
			T m_value;
	};
}

namespace Nz
{
	template<typename T> bool Serialize(SerializationContext& context, ewn::CompressedSigned<T> value, TypeTag<ewn::CompressedSigned<T>>);
	template<typename T> bool Serialize(SerializationContext& context, ewn::CompressedUnsigned<T> value, TypeTag<ewn::CompressedUnsigned<T>>);
	template<typename T> bool Unserialize(SerializationContext& context, ewn::CompressedSigned<T>* value, TypeTag<ewn::CompressedSigned<T>>);
	template<typename T> bool Unserialize(SerializationContext& context, ewn::CompressedUnsigned<T>* value, TypeTag<ewn::CompressedUnsigned<T>>);
}

#include <Shared/Protocol/CompressedInteger.inl>

#endif // EREWHON_SHARED_NETWORK_COMPRESSEDINTEGER_HPP
