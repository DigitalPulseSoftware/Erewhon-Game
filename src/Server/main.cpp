// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerApplication.hpp>
#include <Server/Components/PlayerControlledComponent.hpp>
#include <Server/Systems/SpaceshipSystem.hpp>
#include <Nazara/Core/Initializer.hpp>
#include <Nazara/Network/Network.hpp>

int main()
{
	Nz::Initializer<Nz::Network> networkInit;

	// Initialize custom components
	Ndk::InitializeComponent<ewn::PlayerControlledComponent>("PlyCtrl");
	Ndk::InitializeSystem<ewn::SpaceshipSystem>();

	ewn::ServerApplication app;

	Nz::IpAddress listenAddress = Nz::IpAddress::AnyIpV4;
	listenAddress.SetPort(2049);

	app.SetupNetwork(100, listenAddress);

	while (app.Run())
	{
	}
}
