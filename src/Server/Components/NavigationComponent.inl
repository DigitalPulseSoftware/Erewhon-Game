// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/NavigationComponent.hpp>
#include <Nazara/Math/Algorithm.hpp>

namespace ewn
{
	inline NavigationComponent::NavigationComponent() :
	m_headingController(1.f, 0.f, 0.6382979f)
	{
	}

	inline void NavigationComponent::AddImpulse(const Nz::Vector3f& thrust, Nz::UInt64 expirationTime)
	{
		Impulsion impulsion;
		impulsion.expirationTime = expirationTime;
		impulsion.thrust = thrust;

		m_impulses.emplace_back(std::move(impulsion));
	}

	inline void NavigationComponent::ClearTarget()
	{
		m_target = std::monostate();
	}

	inline NavigationComponent::NavigationResults NavigationComponent::Run(Nz::UInt64 currentTime, float elapsedTime, const Nz::Vector3f& position, const Nz::Quaternionf& rotation, const Nz::Vector3f& linearVel, const Nz::Vector3f& angularVel)
	{
		Nz::Vector3f thrustForce = Nz::Vector3f::Zero();
		Nz::Vector3f rotationForce = Nz::Vector3f::Zero();

		for (auto it = m_impulses.begin(); it != m_impulses.end();)
		{
			if (currentTime < it->expirationTime)
			{
				thrustForce += it->thrust;

				++it;
			}
			else
				it = m_impulses.erase(it);
		}

		auto [linearForce, angularForce, isClose] = ComputeMovement(elapsedTime, position, rotation, linearVel, angularVel);

		thrustForce += linearForce;
		rotationForce += angularForce;

		thrustForce.x = Nz::Clamp(thrustForce.x, -1.f, 1.f);
		thrustForce.y = Nz::Clamp(thrustForce.y, -1.f, 1.f);
		thrustForce.z = Nz::Clamp(thrustForce.z, -1.f, 1.f);
		rotationForce.x = Nz::Clamp(rotationForce.x, -1.f, 1.f);
		rotationForce.y = Nz::Clamp(rotationForce.y, -1.f, 1.f);
		rotationForce.z = Nz::Clamp(rotationForce.z, -1.f, 1.f);

		if (isClose && m_proximityCallback)
		{
			m_proximityCallback();
			m_proximityCallback = nullptr;
		}

		return { thrustForce, rotationForce, isClose };
	}

	inline void NavigationComponent::SetTarget(const Ndk::EntityHandle& entity)
	{
		SetTarget(entity, 0.f, nullptr);
	}

	inline void NavigationComponent::SetTarget(const Ndk::EntityHandle& entity, float triggerDistance, ProximityCallback proximityCallback)
	{
		m_target = entity;
		m_triggerDistance = triggerDistance;
		m_proximityCallback = std::move(proximityCallback);
	}

	inline void NavigationComponent::SetTarget(const Nz::Vector3f& position)
	{
		SetTarget(position, 0.f, nullptr);
	}

	inline void NavigationComponent::SetTarget(const Nz::Vector3f& position, float triggerDistance, ProximityCallback proximityCallback)
	{
		m_target = position;
		m_triggerDistance = triggerDistance;
		m_proximityCallback = std::move(proximityCallback);
	}
}
