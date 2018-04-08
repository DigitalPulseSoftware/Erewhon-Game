// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_SOUNDEMITTERCOMPONENT_HPP
#define EREWHON_CLIENT_SOUNDEMITTERCOMPONENT_HPP

#include <Nazara/Audio/Sound.hpp>
#include <NDK/Component.hpp>
#include <NDK/EntityList.hpp>
#include <optional>

namespace ewn
{
	class ServerApplication;
	class SoundEmitterSystem;

	class SoundEmitterComponent : public Ndk::Component<SoundEmitterComponent>, public Nz::Sound
	{
		friend SoundEmitterSystem;

		public:
			SoundEmitterComponent() = default;
			SoundEmitterComponent(const SoundEmitterComponent&) = default;
			~SoundEmitterComponent() = default;

			static Ndk::ComponentIndex componentIndex;

		private:
			inline const Nz::Vector3f& GetLastPosition() const;
			inline void UpdateLastPosition(const Nz::Vector3f& position);

			Nz::Vector3f m_lastPosition;
	};
}

#include <Client/Components/SoundEmitterComponent.inl>

#endif // EREWHON_CLIENT_SOUNDEMITTERCOMPONENT_HPP
