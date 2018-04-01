// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_STATEDATA_HPP
#define EREWHON_CLIENT_STATES_STATEDATA_HPP

#include <Client/ClientApplication.hpp>
#include <Nazara/Renderer/RenderWindow.hpp>
#include <NDK/Canvas.hpp>
#include <NDK/Entity.hpp>
#include <NDK/World.hpp>

namespace ewn
{
	class ClientApplication;
	class ServerConnection;
	
	struct StateData
	{
		ClientApplication* app;
		ServerConnection* server;
		Nz::RenderWindow* window;
		Ndk::Canvas* canvas;
		Ndk::EntityHandle camera2D;
		Ndk::EntityHandle camera3D;
		Ndk::WorldHandle world2D;
		Ndk::WorldHandle world3D;
	};
}


#endif // EREWHON_CLIENT_STATES_STATEDATA_HPP
