// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_PLASMABEAMWEAPONMODULE_HPP
#define EREWHON_SERVER_PLASMABEAMWEAPONMODULE_HPP

#include <Server/Modules/WeaponModule.hpp>

namespace ewn
{
	class PlasmaBeamWeaponModule final : public WeaponModule
	{
		public:
			using WeaponModule::WeaponModule;
			~PlasmaBeamWeaponModule() = default;

		private:
			void DoShoot() override;
	};
}

#include <Server/Modules/PlasmaBeamWeaponModule.inl>

#endif // EREWHON_SERVER_PLASMABEAMWEAPONMODULE_HPP
