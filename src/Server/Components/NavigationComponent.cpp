// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/NavigationComponent.hpp>
#include <Shared/Utils.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <type_traits>

namespace ewn
{
	NavigationComponent::NavigationResults NavigationComponent::ComputeMovement(float elapsedTime, const Nz::Vector3f& position, const Nz::Quaternionf& rotation, const Nz::Vector3f& linearVel, const Nz::Vector3f& angularVel)
	{
		Nz::Vector3f targetPos;
		bool hasTarget;
		std::visit([&](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, Nz::Vector3f>)
			{
				targetPos = arg;
				hasTarget = true;
			}
			else if constexpr (std::is_same_v<T, Ndk::EntityHandle>)
			{
				if (!arg)
				{
					hasTarget = false;
					return;
				}

				targetPos = arg->template GetComponent<Ndk::NodeComponent>().GetPosition();
				hasTarget = true;
			}
			else if constexpr (std::is_same_v<T, std::monostate>)
				hasTarget = false;
			else
				static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

		}, m_target);

		if (!hasTarget)
			return { Nz::Vector3f::Zero(), Nz::Vector3f::Zero() };

		Nz::Vector3f desiredHeading = targetPos - position;
		bool isCloseEnough = (desiredHeading.GetSquaredLength() <= m_triggerDistance * m_triggerDistance);

		desiredHeading.Normalize();

		Nz::Vector3f currentHeading = rotation * Nz::Vector3f::Forward();
		Nz::Vector3f headingError = currentHeading.CrossProduct(desiredHeading);

		Nz::Vector3f torque = m_headingController.Update(headingError, elapsedTime);

		Nz::Vector3f force = Nz::Vector3f::Zero();
		if (currentHeading.DotProduct(desiredHeading) > 0.95)
			force = Nz::Vector3f::Forward();

		return { force, torque, isCloseEnough };
	}

	Ndk::ComponentIndex NavigationComponent::componentIndex;
}
