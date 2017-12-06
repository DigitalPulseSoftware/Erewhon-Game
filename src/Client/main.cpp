// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Nazara/Core/Directory.hpp>
#include <Nazara/Core/Initializer.hpp>
#include <Nazara/Renderer/RenderWindow.hpp>
#include <Nazara/Network/Network.hpp>
#include <NDK/Canvas.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Systems/RenderSystem.hpp>
#include <NDK/StateMachine.hpp>
#include <Client/ClientApplication.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/ConnectionState.hpp>
#include <Client/States/DisconnectionState.hpp>
#include <Client/States/GameState.hpp>
#include <Client/ServerConnection.hpp>
#include <iostream>

int main()
{
	Nz::Initializer<Nz::Network> networkInit;
	ewn::ClientApplication app;
	app.EnableFPSCounter(true);
	app.SetupNetwork(1, Nz::IpAddress::LoopbackIpV4);

	ewn::ServerConnection serverConnection(app);
	serverConnection.Connect("localhost");
	//serverConnection.Connect("malcolm.digitalpulsesoftware.net");

	Nz::RenderWindow& window = app.AddWindow<Nz::RenderWindow>(Nz::VideoMode(1280, 720), "Utopia");
	window.EnableCloseOnQuit(false);

	// 3D Scene
	Ndk::World& world3D = app.AddWorld();

	const Ndk::EntityHandle& camera3D = world3D.CreateEntity();

	auto& cameraComponent3D = camera3D->AddComponent<Ndk::CameraComponent>();
	cameraComponent3D.SetTarget(&window);
	cameraComponent3D.SetZFar(10'000.f);
	cameraComponent3D.SetZNear(1.f);

	camera3D->AddComponent<Ndk::NodeComponent>();

	// 2D Scene
	Ndk::World& world2D = app.AddWorld();
	world2D.GetSystem<Ndk::RenderSystem>().SetDefaultBackground(nullptr);
	world2D.GetSystem<Ndk::RenderSystem>().SetGlobalUp(Nz::Vector3f::Down());

	const Ndk::EntityHandle& camera2D = world2D.CreateEntity();

	auto& cameraComponent2D = camera2D->AddComponent<Ndk::CameraComponent>();
	cameraComponent2D.SetProjectionType(Nz::ProjectionType_Orthogonal);
	cameraComponent2D.SetTarget(&window);

	camera2D->AddComponent<Ndk::NodeComponent>();

	Ndk::Canvas canvas(world2D.CreateHandle(), window.GetEventHandler(), window.GetCursorController().CreateHandle());
	canvas.SetSize(Nz::Vector2f(window.GetSize()));

	// Resources

	// Loading skybox
	if (Nz::Directory::Exists("Assets/purple_nebula_skybox"))
	{
		Nz::TextureRef background = Nz::Texture::New();
		if (background->Create(Nz::ImageType_Cubemap, Nz::PixelFormatType_RGBA8, 2048, 2048))
		{
			background->LoadFaceFromFile(Nz::CubemapFace_PositiveX, "Assets/purple_nebula_skybox/purple_nebula_skybox_right1.png");
			background->LoadFaceFromFile(Nz::CubemapFace_PositiveY, "Assets/purple_nebula_skybox/purple_nebula_skybox_top3.png");
			background->LoadFaceFromFile(Nz::CubemapFace_PositiveZ, "Assets/purple_nebula_skybox/purple_nebula_skybox_front5.png");
			background->LoadFaceFromFile(Nz::CubemapFace_NegativeX, "Assets/purple_nebula_skybox/purple_nebula_skybox_left2.png");
			background->LoadFaceFromFile(Nz::CubemapFace_NegativeY, "Assets/purple_nebula_skybox/purple_nebula_skybox_bottom4.png");
			background->LoadFaceFromFile(Nz::CubemapFace_NegativeZ, "Assets/purple_nebula_skybox/purple_nebula_skybox_back6.png");
		}

		Nz::TextureLibrary::Register("Background", std::move(background));
	}

	// Text material
	Nz::MaterialRef textMaterial = Nz::Material::New("Translucent2D");
	textMaterial->EnableDepthBuffer(false);

	Nz::MaterialLibrary::Register("SpaceshipText", std::move(textMaterial));

	ewn::StateData stateData;
	stateData.app = &app;
	stateData.camera2D = camera2D;
	stateData.camera3D = camera3D;
	stateData.canvas = &canvas;
	stateData.server = &serverConnection;
	stateData.window = &window;
	stateData.world2D = world2D.CreateHandle();
	stateData.world3D = world3D.CreateHandle();

	Ndk::StateMachine fsm(std::make_shared<ewn::BackgroundState>(stateData));
	fsm.PushState(std::make_shared<ewn::ConnectionState>(stateData));

	window.GetEventHandler().OnQuit.Connect([&](const Nz::EventHandler*)
	{
		fsm.ResetState(std::make_shared<ewn::BackgroundState>(stateData));
		fsm.PushState(std::make_shared<ewn::DisconnectionState>(stateData));
	});

	while (app.Run())
	{
		fsm.Update(app.GetUpdateTime());

		window.Display();
	}
}
