// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_INPUTSYSTEM_HPP
#define EREWHON_SERVER_INPUTSYSTEM_HPP

#include <NDK/System.hpp>

namespace ewn
{
	class InputSystem : public Ndk::System<InputSystem>
	{
		public:
			InputSystem();

			static Ndk::SystemIndex systemIndex;

		private:
			void OnUpdate(float elapsedTime) override;
	};
}

#include <Server/Systems/InputSystem.inl>

#endif // EREWHON_SERVER_INPUTSYSTEM_HPP
