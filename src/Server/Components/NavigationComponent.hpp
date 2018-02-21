// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_NAVIGATIONCOMPONENT_HPP
#define EREWHON_SERVER_NAVIGATIONCOMPONENT_HPP

#include <Nazara/Math/Quaternion.hpp>
#include <Nazara/Math/Vector3.hpp>
#include <NDK/Component.hpp>
#include <Shared/Utils/PidController.hpp>
#include <functional>
#include <variant>
#include <vector>

namespace ewn
{
	class NavigationComponent : public Ndk::Component<NavigationComponent>
	{
		public:
			using ProximityCallback = std::function<void()>;
			struct NavigationResults;

			inline NavigationComponent();
			~NavigationComponent() = default;

			inline void AddImpulse(const Nz::Vector3f& thrust, Nz::UInt64 expirationTime);

			inline void ClearTarget();

			inline NavigationResults Run(Nz::UInt64 currentTime, float elapsedTime, const Nz::Vector3f& position, const Nz::Quaternionf& rotation, const Nz::Vector3f& linearVel, const Nz::Vector3f& angularVel);

			inline void SetTarget(const Ndk::EntityHandle& entity);
			inline void SetTarget(const Ndk::EntityHandle& entity, float triggerDistance, ProximityCallback proximityCallback);
			inline void SetTarget(const Nz::Vector3f& position);
			inline void SetTarget(const Nz::Vector3f& position, float triggerDistance, ProximityCallback proximityCallback);

			static Ndk::ComponentIndex componentIndex;

			struct NavigationResults
			{
				Nz::Vector3f thrust;
				Nz::Vector3f rotation;
				bool triggerDistance;
			};

		private:
			NavigationResults ComputeMovement(float elapsedTime, const Nz::Vector3f& position, const Nz::Quaternionf& rotation, const Nz::Vector3f& linearVel, const Nz::Vector3f& angularVel);

			struct Impulsion
			{
				Nz::UInt64 expirationTime;
				Nz::Vector3f thrust;
			};

			ProximityCallback m_proximityCallback;
			PidController<Nz::Vector3f> m_headingController;
			std::variant<std::monostate, Ndk::EntityHandle, Nz::Vector3f> m_target;
			std::vector<Impulsion> m_impulses;
			float m_triggerDistance;
	};
}

#include <Server/Components/NavigationComponent.inl>

#endif // EREWHON_SERVER_NAVIGATIONCOMPONENT_HPP
