// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Modules/PlasmaBeamWeaponModule.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <Server/Components/ArenaComponent.hpp>
#include <Server/Components/OwnerComponent.hpp>

namespace ewn
{
	void PlasmaBeamWeaponModule::DoShoot()
	{
		const Ndk::EntityHandle& spaceship = GetSpaceship();
		auto& spaceshipNode = spaceship->GetComponent<Ndk::NodeComponent>();
		Arena& spaceshipArena = spaceship->GetComponent<ewn::ArenaComponent>();

		Player* owner = nullptr;
		if (spaceship->HasComponent<OwnerComponent>())
			owner = spaceship->GetComponent<OwnerComponent>().GetOwner();

		spaceshipArena.CreatePlasmaProjectile(owner, spaceship, spaceshipNode.GetPosition() + spaceshipNode.GetForward() * 12.f, spaceshipNode.GetRotation());

		Packets::PlaySound playSound;
		playSound.position = spaceshipNode.GetPosition();
		playSound.soundId = 0;

		spaceshipArena.BroadcastPacket(playSound);
	}
}
