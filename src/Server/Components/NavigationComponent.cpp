// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/NavigationComponent.hpp>
#include <Shared/Utils.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <type_traits>

namespace ewn
{
	std::pair<Nz::Vector3f /*thrust*/, Nz::Vector3f /*rotation*/> NavigationComponent::ComputeMovement(float elapsedTime, const Nz::Vector3f& position, const Nz::Quaternionf& rotation, const Nz::Vector3f& linearVel, const Nz::Vector3f& angularVel)
{
		Nz::Vector3f targetPos;
		std::visit([&targetPos](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, Nz::Vector3f>)
				targetPos = arg;
			else if constexpr (std::is_same_v<T, Ndk::EntityHandle>)
				targetPos = arg->GetComponent<Ndk::NodeComponent>().GetPosition();
			else if constexpr (std::is_same_v<T, std::monostate>)
				assert(false); //< ComputeMovement is not supposed to be called when there's no target
			else
				static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

		}, m_target);

		Nz::Vector3f desiredHeading = targetPos - position;
		desiredHeading.Normalize();

		Nz::Vector3f currentHeading = rotation * Nz::Vector3f::Forward();
		Nz::Vector3f headingError = currentHeading.CrossProduct(desiredHeading);

		Nz::Vector3f torque = m_headingController.Update(headingError, elapsedTime);

		return { Nz::Vector3f::Zero(), torque };
	}

	Ndk::ComponentIndex NavigationComponent::componentIndex;
}
