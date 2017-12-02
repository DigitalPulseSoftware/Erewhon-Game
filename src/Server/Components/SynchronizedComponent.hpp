// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_SYNCHRONIZEDCOMPONENT_HPP
#define EREWHON_SERVER_SYNCHRONIZEDCOMPONENT_HPP

#include <NDK/Component.hpp>
#include <string>

namespace ewn
{
	class SynchronizedComponent : public Ndk::Component<SynchronizedComponent>
	{
		public:
			inline SynchronizedComponent(std::string type, std::string nameTemp, bool movable);

			inline const std::string& GetName() const;
			inline const std::string& GetType() const;
			inline bool IsMovable() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			std::string m_name;
			std::string m_type;
			bool m_movable;
	};
}

#include <Server/Components/SynchronizedComponent.inl>

#endif // EREWHON_SERVER_SYNCHRONIZEDCOMPONENT_HPP
