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
			inline SynchronizedComponent(std::size_t prefabId, std::string type, std::string nameTemp, bool movable, Nz::UInt16 networkPriority);

			inline void AccumulatePriority();

			inline const std::string& GetName() const;
			inline std::size_t GetPrefabId() const;
			inline Nz::UInt16 GetPriority() const;
			inline Nz::UInt16 GetPriorityAccumulator() const;
			inline const std::string& GetType() const;

			inline bool IsMovable() const;

			inline void ResetPriorityAccumulator();

			static Ndk::ComponentIndex componentIndex;

		private:
			std::size_t m_prefabId;
			std::string m_name;
			std::string m_type;
			Nz::UInt16 m_priority;
			Nz::UInt16 m_priorityAccumulator;
			bool m_movable;
	};
}

#include <Server/Components/SynchronizedComponent.inl>

#endif // EREWHON_SERVER_SYNCHRONIZEDCOMPONENT_HPP
