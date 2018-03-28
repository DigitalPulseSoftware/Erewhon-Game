// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_TORPEDOBEAMWEAPONMODULE_HPP
#define EREWHON_SERVER_TORPEDOBEAMWEAPONMODULE_HPP

#include <Server/Modules/WeaponModule.hpp>

namespace ewn
{
	class TorpedoWeaponModule final : public WeaponModule
	{
		public:
			using WeaponModule::WeaponModule;
			~TorpedoWeaponModule() = default;

		private:
			void DoShoot() override;
	};
}

#include <Server/Modules/PlasmaBeamWeaponModule.inl>

#endif // EREWHON_SERVER_TORPEDOBEAMWEAPONMODULE_HPP
