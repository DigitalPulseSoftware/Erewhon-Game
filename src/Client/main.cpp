// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Nazara/Audio/Audio.hpp>
#include <Nazara/Audio/SoundBuffer.hpp>
#include <Nazara/Core/Directory.hpp>
#include <Nazara/Core/Initializer.hpp>
#include <Nazara/Graphics/DeferredRenderTechnique.hpp>
#include <Nazara/Renderer/RenderWindow.hpp>
#include <Nazara/Network/Network.hpp>
#include <NDK/Canvas.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/ListenerComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Systems/RenderSystem.hpp>
#include <NDK/StateMachine.hpp>
#include <Client/ClientApplication.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/DisconnectionState.hpp>
#include <Client/States/LoginState.hpp>
#include <Client/ServerConnection.hpp>
#include <iostream>

int main()
{
	Nz::Initializer<Nz::Audio, Nz::Network> nazaraInit;

	ewn::ClientApplication app;
	if (!app.LoadConfig("cconfig.lua"))
	{
		std::cerr << "Failed to load config file" << std::endl;
		return EXIT_FAILURE;
	}

	const ewn::ConfigFile& config = app.GetConfig();

	app.EnableFPSCounter(true);

	ewn::ServerConnection serverConnection(app);

	bool fullscreen = config.GetBoolOption("Options.Fullscreen");
	Nz::RenderWindow& window = app.AddWindow<Nz::RenderWindow>((fullscreen) ? Nz::VideoMode::GetFullscreenModes()[0] : Nz::VideoMode(1280, 720), "Utopia", (fullscreen) ? Nz::WindowStyle_Fullscreen : Nz::WindowStyle_Default);
	window.EnableCloseOnQuit(false);
	window.EnableVerticalSync(config.GetBoolOption("Options.VerticalSync"));

	// 3D Scene
	Ndk::World& world3D = app.AddWorld();
	//world3D.GetSystem<Ndk::RenderSystem>().ChangeRenderTechnique(std::make_unique<Nz::DeferredRenderTechnique>());

	const Ndk::EntityHandle& camera3D = world3D.CreateEntity();

	auto& cameraComponent3D = camera3D->AddComponent<Ndk::CameraComponent>();
	cameraComponent3D.SetTarget(&window);
	cameraComponent3D.SetZFar(10'000.f);
	cameraComponent3D.SetZNear(1.f);

	//camera3D->AddComponent<Ndk::ListenerComponent>();
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
	std::string assetsFolder = config.GetStringOption("AssetsFolder");

	// Loading skybox
	if (Nz::Directory::Exists(assetsFolder + "/purple_nebula_skybox"))
	{
		Nz::TextureRef background = Nz::Texture::New();
		if (background->Create(Nz::ImageType_Cubemap, Nz::PixelFormatType_RGBA8, 2048, 2048))
		{
			background->LoadFaceFromFile(Nz::CubemapFace_PositiveX, assetsFolder + "/purple_nebula_skybox/purple_nebula_skybox_right1.png");
			background->LoadFaceFromFile(Nz::CubemapFace_PositiveY, assetsFolder + "/purple_nebula_skybox/purple_nebula_skybox_top3.png");
			background->LoadFaceFromFile(Nz::CubemapFace_PositiveZ, assetsFolder + "/purple_nebula_skybox/purple_nebula_skybox_front5.png");
			background->LoadFaceFromFile(Nz::CubemapFace_NegativeX, assetsFolder + "/purple_nebula_skybox/purple_nebula_skybox_left2.png");
			background->LoadFaceFromFile(Nz::CubemapFace_NegativeY, assetsFolder + "/purple_nebula_skybox/purple_nebula_skybox_bottom4.png");
			background->LoadFaceFromFile(Nz::CubemapFace_NegativeZ, assetsFolder + "/purple_nebula_skybox/purple_nebula_skybox_back6.png");
		}

		Nz::TextureLibrary::Register("Background", std::move(background));
	}

	// Shoot sound
	Nz::SoundBufferParams soundParams;
	soundParams.forceMono = true;

	Nz::SoundBufferRef shootSound = Nz::SoundBuffer::New();
	if (!shootSound->LoadFromFile(assetsFolder + "/sounds/laserTurretlow.ogg", soundParams))
		std::cerr << "Failed to load shoot sound" << std::endl;

	Nz::SoundBufferLibrary::Register("ShootSound", std::move(shootSound));

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
	fsm.PushState(std::make_shared<ewn::LoginState>(stateData));

	// Handle exit
	window.GetEventHandler().OnQuit.Connect([&](const Nz::EventHandler*)
	{
		fsm.ResetState(std::make_shared<ewn::BackgroundState>(stateData));
		fsm.PushState(std::make_shared<ewn::DisconnectionState>(stateData));
	});

	// Handle size change
	window.GetEventHandler().OnResized.Connect([&](const Nz::EventHandler*, const Nz::WindowEvent::SizeEvent& sizeEvent)
	{
		canvas.SetSize(Nz::Vector2f(Nz::Vector2ui(sizeEvent.width, sizeEvent.height)));
	});

	while (app.Run())
	{
		fsm.Update(app.GetUpdateTime());

		window.Display();
	}
}
