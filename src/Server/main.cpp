// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerApplication.hpp>
#include <Server/Components/ArenaComponent.hpp>
#include <Server/Components/HealthComponent.hpp>
#include <Server/Components/InputComponent.hpp>
#include <Server/Components/LifeTimeComponent.hpp>
#include <Server/Components/NavigationComponent.hpp>
#include <Server/Components/OwnerComponent.hpp>
#include <Server/Components/PlayerControlledComponent.hpp>
#include <Server/Components/ProjectileComponent.hpp>
#include <Server/Components/RadarComponent.hpp>
#include <Server/Components/ScriptComponent.hpp>
#include <Server/Components/SynchronizedComponent.hpp>
#include <Server/Systems/BroadcastSystem.hpp>
#include <Server/Systems/LifeTimeSystem.hpp>
#include <Server/Systems/NavigationSystem.hpp>
#include <Server/Systems/ScriptSystem.hpp>
#include <Server/Systems/SpaceshipSystem.hpp>
#include <Nazara/Core/Initializer.hpp>
#include <Nazara/Core/Thread.hpp>
#include <Nazara/Network/Network.hpp>
#include <NDK/Sdk.hpp>

int main()
{
	Nz::Initializer<Nz::Network, Ndk::Sdk> nazara; //< Init SDK before application because of custom components/systems

	// Initialize custom components
	Ndk::InitializeComponent<ewn::ArenaComponent>("Arena");
	Ndk::InitializeComponent<ewn::HealthComponent>("Health");
	Ndk::InitializeComponent<ewn::LifeTimeComponent>("LifeTime");
	Ndk::InitializeComponent<ewn::InputComponent>("InptComp");
	Ndk::InitializeComponent<ewn::NavigationComponent>("NavigCmp");
	Ndk::InitializeComponent<ewn::OwnerComponent>("OwnrComp");
	Ndk::InitializeComponent<ewn::PlayerControlledComponent>("PlyCtrl");
	Ndk::InitializeComponent<ewn::ProjectileComponent>("Prjctile");
	Ndk::InitializeComponent<ewn::RadarComponent>("RadarCmp");
	Ndk::InitializeComponent<ewn::ScriptComponent>("ScrptCmp");
	Ndk::InitializeComponent<ewn::SynchronizedComponent>("SyncComp");
	Ndk::InitializeSystem<ewn::BroadcastSystem>();
	Ndk::InitializeSystem<ewn::LifeTimeSystem>();
	Ndk::InitializeSystem<ewn::NavigationSystem>();
	Ndk::InitializeSystem<ewn::ScriptSystem>();
	Ndk::InitializeSystem<ewn::SpaceshipSystem>();

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

	const ewn::ConfigFile& config = app.GetConfig();
	
	Nz::IpAddress listenAddress = Nz::IpAddress::AnyIpV4;
	listenAddress.SetPort(config.GetIntegerOption<Nz::UInt16>("Game.Port"));

	if (!app.SetupNetwork(config.GetIntegerOption<std::size_t>("Game.MaxClients"), listenAddress))
	{
		std::cerr << "Failed to setup network" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Server ready." << std::endl;

	while (app.Run())
	{
		Nz::Thread::Sleep(1);
	}

	std::cout << "Goodbye" << std::endl;
}
