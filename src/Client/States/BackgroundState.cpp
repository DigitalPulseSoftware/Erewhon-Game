// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/BackgroundState.hpp>
#include <Nazara/Core/Directory.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <Nazara/Graphics/SkyboxBackground.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Systems/RenderSystem.hpp>
#include <NDK/Entity.hpp>
#include <random>

namespace ewn
{
	void BackgroundState::Enter(Ndk::StateMachine& /*fsm*/)
	{
		StateData& stateData = GetStateData();

		if (Nz::Texture* background = Nz::TextureLibrary::Get("Background"); background && background->IsValid())
			stateData.world3D->GetSystem<Ndk::RenderSystem>().SetDefaultBackground(Nz::SkyboxBackground::New(background));
		else
			stateData.world3D->GetSystem<Ndk::RenderSystem>().SetDefaultBackground(Nz::ColorBackground::New(Nz::Color::Black));

		std::random_device rd;
		std::uniform_real_distribution<float> rotGen(-180.f, 180.f);

		Ndk::NodeComponent& nodeComponent = stateData.camera3D->GetComponent<Ndk::NodeComponent>();
		nodeComponent.SetRotation(Nz::EulerAnglesf(rotGen(rd), rotGen(rd), rotGen(rd)));
	}

	bool BackgroundState::Update(Ndk::StateMachine& /*fsm*/, float elapsedTime)
	{
		Ndk::NodeComponent& nodeComponent = GetStateData().camera3D->GetComponent<Ndk::NodeComponent>();
		nodeComponent.Rotate(Nz::EulerAnglesf(elapsedTime, elapsedTime * 1.5f, 0.f));

		return true;
	}
}
