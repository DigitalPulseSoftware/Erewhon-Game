// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/BackgroundState.hpp>
#include <Nazara/Core/Directory.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <Nazara/Graphics/ForwardRenderTechnique.hpp>
#include <Nazara/Graphics/SkyboxBackground.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Systems/RenderSystem.hpp>
#include <NDK/Entity.hpp>

namespace ewn
{
	BackgroundState::BackgroundState(StateData& stateData) :
	m_stateData(stateData)
	{
		// Loading skybox
		if (Nz::Directory::Exists("assets/purple_nebula_skybox"))
		{
			m_backgroundCubemap = Nz::Texture::New();
			if (m_backgroundCubemap->Create(Nz::ImageType_Cubemap, Nz::PixelFormatType_RGBA8, 2048, 2048))
			{
				m_backgroundCubemap->LoadFaceFromFile(Nz::CubemapFace_PositiveX, "assets/purple_nebula_skybox/purple_nebula_skybox_right1.png");
				m_backgroundCubemap->LoadFaceFromFile(Nz::CubemapFace_PositiveY, "assets/purple_nebula_skybox/purple_nebula_skybox_top3.png");
				m_backgroundCubemap->LoadFaceFromFile(Nz::CubemapFace_PositiveZ, "assets/purple_nebula_skybox/purple_nebula_skybox_front5.png");
				m_backgroundCubemap->LoadFaceFromFile(Nz::CubemapFace_NegativeX, "assets/purple_nebula_skybox/purple_nebula_skybox_left2.png");
				m_backgroundCubemap->LoadFaceFromFile(Nz::CubemapFace_NegativeY, "assets/purple_nebula_skybox/purple_nebula_skybox_bottom4.png");
				m_backgroundCubemap->LoadFaceFromFile(Nz::CubemapFace_NegativeZ, "assets/purple_nebula_skybox/purple_nebula_skybox_back6.png");
			}
		}
	}

	void BackgroundState::Enter(Ndk::StateMachine& /*fsm*/)
	{
		if (m_backgroundCubemap)
			m_stateData.world3D->GetSystem<Ndk::RenderSystem>().SetDefaultBackground(Nz::SkyboxBackground::New(m_backgroundCubemap));
		else
			m_stateData.world3D->GetSystem<Ndk::RenderSystem>().SetDefaultBackground(Nz::ColorBackground::New(Nz::Color::Black));
	}

	void BackgroundState::Leave(Ndk::StateMachine& /*fsm*/)
	{
	}

	bool BackgroundState::Update(Ndk::StateMachine& /*fsm*/, float elapsedTime)
	{
		Ndk::NodeComponent& nodeComponent = m_stateData.camera3D->GetComponent<Ndk::NodeComponent>();
		nodeComponent.Rotate(Nz::EulerAnglesf(elapsedTime, elapsedTime * 1.5f, 0.f));

		return true;
	}
}
