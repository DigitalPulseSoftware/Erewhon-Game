// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/InputComponent.hpp>

namespace ewn
{
	inline InputComponent::InputComponent() :
	m_lastInputTime(0)
	{
	}

	inline Nz::UInt64 InputComponent::GetLastInputTime() const
	{
		return m_lastInputTime;
	}

	template<typename F>
	void InputComponent::ProcessInputs(F inputFunc)
	{
		for (const InputData& input : m_inputs)
			inputFunc(input.serverTime, input.direction, input.rotation);

		if (!m_inputs.empty())
		{
			m_lastInputTime = m_inputs.back().serverTime;
			m_inputs.clear();
		}
	}

	inline void InputComponent::PushInput(Nz::UInt64 inputTime, const Nz::Vector3f& movement, const Nz::Vector3f& rotation)
	{
		assert(inputTime > m_lastInputTime);
		assert(movement.x >= -1.f && movement.x <= 1.f);
		assert(movement.y >= -1.f && movement.y <= 1.f);
		assert(movement.z >= -1.f && movement.z <= 1.f);
		assert(rotation.x >= -1.f && rotation.x <= 1.f);
		assert(rotation.y >= -1.f && rotation.y <= 1.f);
		assert(rotation.z >= -1.f && rotation.z <= 1.f);

		InputData inputData;
		inputData.serverTime = inputTime;
		inputData.direction = movement * 50.f;
		inputData.rotation = rotation * 200.f;

		m_inputs.emplace_back(std::move(inputData));
	}
}
