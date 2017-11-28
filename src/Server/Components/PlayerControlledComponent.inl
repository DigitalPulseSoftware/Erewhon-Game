// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Components/PlayerControlledComponent.hpp>

namespace ewn
{
	inline PlayerControlledComponent::PlayerControlledComponent() :
	m_lastInputTime(0)
	{
	}

	inline Nz::UInt64 PlayerControlledComponent::GetLastInputTime() const
	{
		return m_lastInputTime;
	}

	template<typename F>
	void PlayerControlledComponent::ProcessInputs(F inputFunc)
	{
		for (const InputData& input : m_inputs)
			inputFunc(input.serverTime, input.direction, input.rotation);

		if (!m_inputs.empty())
		{
			m_lastInputTime = m_inputs.back().serverTime;
			m_inputs.clear();
		}
	}

	inline void PlayerControlledComponent::PushInput(Nz::UInt64 inputTime, const Nz::Vector3f& direction, const Nz::Vector3f& rotation)
	{
		assert(inputTime > m_lastInputTime);

		InputData inputData;
		inputData.serverTime = inputTime;
		inputData.direction = direction;
		inputData.rotation = rotation;

		m_inputs.emplace_back(std::move(inputData));
	}
}
