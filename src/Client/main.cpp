// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

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
#include <Client/States/LoginState.hpp>
#include <iostream>

int main()
{
	Nz::Initializer<Nz::Network> networkInit;
	ewn::ClientApplication app;
	app.SetupNetwork(1, Nz::IpAddress::LoopbackIpV4);

	app.Connect("localhost");

	Nz::RenderWindow& window = app.AddWindow<Nz::RenderWindow>(Nz::VideoMode(1280, 720), "Utopia");

	// 3D Scene
	Ndk::World& world3D = app.AddWorld();

	const Ndk::EntityHandle& camera3D = world3D.CreateEntity();

	auto& cameraComponent3D = camera3D->AddComponent<Ndk::CameraComponent>();
	cameraComponent3D.SetTarget(&window);

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

	ewn::StateData stateData;
	stateData.app = &app;
	stateData.camera2D = camera2D;
	stateData.camera3D = camera3D;
	stateData.canvas = &canvas;
	stateData.window = &window;
	stateData.world2D = world2D.CreateHandle();
	stateData.world3D = world3D.CreateHandle();

	Ndk::StateMachine fsm(std::make_shared<ewn::BackgroundState>(stateData));
	fsm.PushState(std::make_shared<ewn::ConnectionState>(stateData));

	while (app.Run())
	{
		fsm.Update(app.GetUpdateTime());

		window.Display();
	}
}
