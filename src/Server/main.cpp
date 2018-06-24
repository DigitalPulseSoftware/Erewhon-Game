// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerApplication.hpp>
#include <Server/Components/ArenaComponent.hpp>
#include <Server/Components/CommunicationComponent.hpp>
#include <Server/Components/HealthComponent.hpp>
#include <Server/Components/InputComponent.hpp>
#include <Server/Components/LifeTimeComponent.hpp>
#include <Server/Components/NavigationComponent.hpp>
#include <Server/Components/OwnerComponent.hpp>
#include <Server/Components/PlayerControlledComponent.hpp>
#include <Server/Components/ProjectileComponent.hpp>
#include <Server/Components/ScriptComponent.hpp>
#include <Server/Components/SignatureComponent.hpp>
#include <Server/Components/SynchronizedComponent.hpp>
#include <Server/Scripting/ArenaInterface.hpp>
#include <Server/Systems/BroadcastSystem.hpp>
#include <Server/Systems/LifeTimeSystem.hpp>
#include <Server/Systems/NavigationSystem.hpp>
#include <Server/Systems/ScriptSystem.hpp>
#include <Server/Systems/InputSystem.hpp>
#include <Nazara/Core/Initializer.hpp>
#include <Nazara/Core/Thread.hpp>
#include <Nazara/Network/Network.hpp>
#include <NDK/Sdk.hpp>

int main()
{
	Nz::Initializer<Nz::Network, Ndk::Sdk> nazara; //< Init SDK before application because of custom components/systems

	Nz::Initializer<ewn::ArenaInterface> binding;

	// Initialize custom components
	Ndk::InitializeComponent<ewn::ArenaComponent>("Arena");
	Ndk::InitializeComponent<ewn::CommunicationComponent>("ComComp");
	Ndk::InitializeComponent<ewn::HealthComponent>("Health");
	Ndk::InitializeComponent<ewn::LifeTimeComponent>("LifeTime");
	Ndk::InitializeComponent<ewn::InputComponent>("InptComp");
	Ndk::InitializeComponent<ewn::NavigationComponent>("NavigCmp");
	Ndk::InitializeComponent<ewn::OwnerComponent>("OwnrComp");
	Ndk::InitializeComponent<ewn::PlayerControlledComponent>("PlyCtrl");
	Ndk::InitializeComponent<ewn::ProjectileComponent>("Prjctile");
	Ndk::InitializeComponent<ewn::ScriptComponent>("ScrptCmp");
	Ndk::InitializeComponent<ewn::SignatureComponent>("SignCmp");
	Ndk::InitializeComponent<ewn::SynchronizedComponent>("SyncComp");
	Ndk::InitializeSystem<ewn::BroadcastSystem>();
	Ndk::InitializeSystem<ewn::LifeTimeSystem>();
	Ndk::InitializeSystem<ewn::NavigationSystem>();
	Ndk::InitializeSystem<ewn::ScriptSystem>();
	Ndk::InitializeSystem<ewn::InputSystem>();

	ewn::ServerApplication app;
	if (!app.LoadConfig("sconfig.lua"))
	{
		std::cerr << "Failed to load config file" << std::endl;
		return EXIT_FAILURE;
	}

	if (!app.LoadDatabase())
	{
		std::cerr << "Failed to load database" << std::endl;
		return EXIT_FAILURE;
	}

	app.CreateArena("L'Arène de trèfle", "arena.lua");
	app.CreateArena("L'Arène d'Angleterre", "arena.lua");
	app.CreateArena("Le bac à sable", "sandbox.lua");

	const ewn::ConfigFile& config = app.GetConfig();
	if (!app.SetupNetwork(config.GetIntegerOption<std::size_t>("Game.MaxClients"), 1, Nz::NetProtocol_Any, config.GetIntegerOption<Nz::UInt16>("Game.Port")))
	{
		std::cerr << "Failed to setup network" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Server ready." << std::endl;

	//Nz::UInt64 lastUpdate = Nz::GetElapsedMilliseconds();
	//unsigned int updateCount = 0;
	while (app.Run())
	{
		/*Nz::UInt64 now = Nz::GetElapsedMilliseconds();
		if (now - lastUpdate >= 1000)
		{
			std::cout << "Update per seconds: " << updateCount << std::endl;
			updateCount = 0;
			lastUpdate = now;
		}
		updateCount++;*/
		Nz::Thread::Sleep(1);
	}

	std::cout << "Goodbye" << std::endl;
}
