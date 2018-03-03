// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_RADARCOMPONENT_HPP
#define EREWHON_SERVER_RADARCOMPONENT_HPP

#include <Nazara/Math/Vector3.hpp>
#include <NDK/Component.hpp>
#include <NDK/Entity.hpp>
#include <unordered_map>

namespace ewn
{
	class RadarComponent : public Ndk::Component<RadarComponent>
	{
		public:
			using OnDestructionCallback = std::function<void(Ndk::Entity*)>;
			using OnLeaveCallback = std::function<void(Ndk::Entity*)>;

			inline RadarComponent();
			RadarComponent(const RadarComponent& radar);
			RadarComponent(RadarComponent&&) = delete;
			~RadarComponent() = default;

			void ClearWatchedTargets();

			void CheckTargetRange(const Nz::Vector3f& position, float maxRange);

			inline void UnwatchEntity(Ndk::Entity* entity);
			inline void WatchEntity(Ndk::Entity* entity, OnDestructionCallback destructionCallback, OnLeaveCallback leaveCallback);

			RadarComponent& operator=(const RadarComponent&) = delete;
			RadarComponent& operator=(RadarComponent&&) = delete;

			static Ndk::ComponentIndex componentIndex;

		private:
			void OnWatchedEntityDestroyed(Ndk::Entity* entity);

			struct WatchedTarget
			{
				OnDestructionCallback destructionCallback;
				OnLeaveCallback rangeLeaveCallback;
				Ndk::EntityHandle target;

				NazaraSlot(Ndk::Entity, OnEntityDestruction, onEntityDestroyed);
			};

			std::unordered_map<Ndk::EntityId, WatchedTarget> m_watchedTargets;
	};
}

#include <Server/Components/RadarComponent.inl>

#endif // EREWHON_SERVER_RADARCOMPONENT_HPP
