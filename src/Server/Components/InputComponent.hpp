// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_INPUTCOMPONENT_HPP
#define EREWHON_SERVER_INPUTCOMPONENT_HPP

#include <Nazara/Math/Vector3.hpp>
#include <NDK/Component.hpp>
#include <Server/Player.hpp>

namespace ewn
{
	class InputComponent : public Ndk::Component<InputComponent>
	{
		public:
			InputComponent();

			inline Nz::UInt64 GetLastInputTime() const;

			template<typename F> void ProcessInputs(F inputFunc);

			inline void PushInput(Nz::UInt64 inputTime, const Nz::Vector3f& direction, const Nz::Vector3f& rotation);

			static Ndk::ComponentIndex componentIndex;

		private:
			struct InputData
			{
				Nz::UInt64 serverTime;
				Nz::Vector3f direction;
				Nz::Vector3f rotation;
			};

			Nz::UInt64 m_lastInputTime;
			std::vector<InputData> m_inputs;
	};
}

#include <Server/Components/InputComponent.inl>

#endif // EREWHON_SERVER_INPUTCOMPONENT_HPP
