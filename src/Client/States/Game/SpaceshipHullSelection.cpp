// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/SpaceshipHullSelection.hpp>
#include <Nazara/Core/String.hpp>
#include <Nazara/Lua/LuaInstance.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/LightComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/StateMachine.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Client/States/Game/SpaceshipEditState.hpp>
#include <cassert>
#include <limits>

constexpr float SpaceshipSize = 128.f;

namespace ewn
{
	void SpaceshipHullSelection::Enter(Ndk::StateMachine& fsm)
	{
		AbstractState::Enter(fsm);

		StateData& stateData = GetStateData();

		const ConfigFile& config = stateData.app->GetConfig();

		m_selectedHullIndex = std::numeric_limits<std::size_t>::max();

		m_backButton = CreateWidget<Ndk::ButtonWidget>();
		//m_backButton->SetPadding(15.f, 15.f, 15.f, 15.f);
		m_backButton->UpdateText(Nz::SimpleTextDrawer::Draw("Back", 24));
		//m_backButton->ResizeToContent();
		m_backButton->OnButtonTrigger.Connect([&](const Ndk::ButtonWidget* /*button*/)
		{
			OnBackPressed();
		});

		m_selectButton = CreateWidget<Ndk::ButtonWidget>();
		//m_selectButton->SetPadding(15.f, 15.f, 15.f, 15.f);
		m_selectButton->UpdateText(Nz::SimpleTextDrawer::Draw("Select hull", 24));
		//m_selectButton->ResizeToContent();
		m_selectButton->OnButtonTrigger.Connect([&](const Ndk::ButtonWidget* /*button*/)
		{
			OnSelectPressed();
		});

		m_descriptionLabel = CreateWidget<Ndk::LabelWidget>();
		m_nameLabel = CreateWidget<Ndk::LabelWidget>();
		m_slotLabel = CreateWidget<Ndk::LabelWidget>();

		m_light = stateData.world3D->CreateEntity();
		m_light->AddComponent<Ndk::LightComponent>(Nz::LightType_Directional);
		auto& lightNode = m_light->AddComponent<Ndk::NodeComponent>();
		lightNode.SetParent(stateData.camera3D);

		m_selectedHull = stateData.world3D->CreateEntity();
		m_selectedHull->AddComponent<Ndk::GraphicsComponent>();
		auto& spaceshipNode = m_selectedHull->AddComponent<Ndk::NodeComponent>();
		spaceshipNode.SetParent(stateData.camera3D);

		LayoutWidgets();

		ConnectSignal(stateData.server->OnHullList, this,  &SpaceshipHullSelection::OnHullList);

		QueryHullList();
	}

	void SpaceshipHullSelection::Leave(Ndk::StateMachine& fsm)
	{
		AbstractState::Leave(fsm);

		m_hullInfo.clear();
		m_light.Reset();
		m_selectedHull.Reset();
	}

	bool SpaceshipHullSelection::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		StateData& stateData = GetStateData();

		for (const auto& hullInfo : m_hullInfo)
			hullInfo.hullEntity->GetComponent<Ndk::NodeComponent>().Rotate(Nz::EulerAnglesf(0.f, 30.f * elapsedTime, 0.f));

		return true;
	}

	void SpaceshipHullSelection::LayoutWidgets()
	{
		StateData& stateData = GetStateData();

		Nz::Vector2f canvasSize = stateData.canvas->GetSize();

		Ndk::CameraComponent& camera3D = stateData.camera3D->GetComponent<Ndk::CameraComponent>();

		m_backButton->SetPosition(20.f, canvasSize.y - m_backButton->GetSize().y - 20.f);

		// Central
		constexpr float selectedHullSize = 400.f; //< approx

		float zPos = camera3D.ProjectDepth(1.f + camera3D.GetZNear()); //< Pifometer way

		float centralCursor = canvasSize.y / 2.f;

		Nz::Vector3f selectHullPos = camera3D.Unproject({ canvasSize.x / 2.f, centralCursor - selectedHullSize / 2.f, zPos });
		m_selectedHull->GetComponent<Ndk::NodeComponent>().SetPosition(selectHullPos, Nz::CoordSys_Global);

		m_nameLabel->SetPosition(canvasSize.x / 2.f - m_nameLabel->GetSize().x / 2.f, centralCursor);
		centralCursor += m_nameLabel->GetSize().y;

		m_descriptionLabel->SetPosition(canvasSize.x / 2.f - m_descriptionLabel->GetSize().x / 2.f, centralCursor);
		centralCursor += m_descriptionLabel->GetSize().y;

		m_selectButton->SetPosition(0.f, canvasSize.y - m_selectButton->GetSize().y - 20.f);
		m_selectButton->CenterHorizontal();

		// Left
		Nz::Vector2f leftCursor = Nz::Vector2f(canvasSize.x * 0.1f, canvasSize.y * 0.2f);
		for (const auto& hullInfo : m_hullInfo)
		{
			auto& nodeComponent = hullInfo.hullEntity->GetComponent<Ndk::NodeComponent>();

			float zPos = camera3D.ProjectDepth(5.f + camera3D.GetZNear()); //< Pifometer way

			Nz::Vector3f worldPos = camera3D.Unproject({ leftCursor.x + SpaceshipSize / 2.f, leftCursor.y + SpaceshipSize / 2.f, zPos });
			nodeComponent.SetPosition(worldPos, Nz::CoordSys_Global);

			hullInfo.button->SetPosition({ leftCursor.x, leftCursor.y + SpaceshipSize, 0.f });
			leftCursor.y += hullInfo.button->GetSize().y + SpaceshipSize + 10.f;
		}

		// Right
		m_slotLabel->SetPosition(canvasSize.x - m_slotLabel->GetSize().x - 20.f, 0.f);
		m_slotLabel->CenterVertical();
	}

	void SpaceshipHullSelection::QueryHullList()
	{
		GetStateData().server->SendPacket(Packets::QueryHullList{});
	}

	void SpaceshipHullSelection::OnBackPressed()
	{
		GetStateData().fsm->ChangeState(m_previousState);
	}

	void SpaceshipHullSelection::OnHullSwitch(std::size_t hullId)
	{
		m_selectedHullIndex = hullId;

		auto& hullGfx = m_selectedHull->GetComponent<Ndk::GraphicsComponent>();

		const auto& hullInfo = m_hullInfo[hullId];

		hullGfx.Clear();

		float boundingRadius = hullInfo.hullModel->GetBoundingVolume().obb.localBox.GetRadius();
		Nz::Matrix4f transformMatrix = Nz::Matrix4f::Scale(Nz::Vector3f::Unit() / boundingRadius);

		hullGfx.Attach(m_hullInfo[hullId].hullModel, transformMatrix);

		m_nameLabel->UpdateText(Nz::SimpleTextDrawer::Draw(hullInfo.hullName, 24, Nz::TextStyle_Bold));
		//m_nameLabel->ResizeToContent();

		m_descriptionLabel->UpdateText(Nz::SimpleTextDrawer::Draw(hullInfo.hullDescription, 18));
		//m_descriptionLabel->ResizeToContent();

		m_slotLabel->UpdateText(Nz::SimpleTextDrawer::Draw(hullInfo.hullSlotsDescription, 20));
		//m_slotLabel->ResizeToContent();

		LayoutWidgets();
	}

	void SpaceshipHullSelection::OnSelectPressed()
	{
		StateData& stateData = GetStateData();
		stateData.fsm->ChangeState(std::make_shared<SpaceshipEditState>(SpaceshipEditState::CreateMode{}, stateData, m_previousState, m_hullInfo[m_selectedHullIndex].hullPath, m_hullInfo[m_selectedHullIndex].hullId));
	}

	void SpaceshipHullSelection::OnHullList(ServerConnection* server, const Packets::HullList& hullList)
	{
		StateData& stateData = GetStateData();

		// Hull buttons
		for (const auto& hullInfo : m_hullInfo)
			DestroyWidget(hullInfo.button);

		m_hullInfo.clear();
		
		const std::string& assetsFolder = server->GetApp().GetConfig().GetStringOption("AssetsFolder");

		for (const auto& hullData : hullList.hulls)
		{
			HullInfo& hullInfo = m_hullInfo.emplace_back();

			// Info
			hullInfo.hullId = hullData.hullId;
			hullInfo.hullName = hullData.name;
			hullInfo.hullDescription = hullData.description;
			hullInfo.hullPath = server->GetNetworkStringStore().GetString(hullData.hullModelPathId);

			// Count modules by type
			static constexpr std::size_t ModuleCount = static_cast<std::size_t>(ModuleType::Max) + 1;

			std::array<std::size_t, ModuleCount> moduleCounts{};
			for (const auto& slot : hullData.slots)
				moduleCounts[static_cast<std::size_t>(slot.type)]++;

			hullInfo.hullSlotsDescription = "Slots:";
			for (std::size_t i = 0; i < ModuleCount; ++i)
			{
				if (moduleCounts[i] > 0)
				{
					hullInfo.hullSlotsDescription += '\n';
					hullInfo.hullSlotsDescription += std::to_string(moduleCounts[i]);
					hullInfo.hullSlotsDescription += "x ";
					hullInfo.hullSlotsDescription += EnumToString(static_cast<ModuleType>(i));
				}
			}

			// Button
			hullInfo.button = CreateWidget<Ndk::ButtonWidget>();
			//hullInfo.button->SetPadding(15.f, 15.f, 15.f, 15.f);
			hullInfo.button->UpdateText(Nz::SimpleTextDrawer::Draw(hullData.name, 18));
			//hullInfo.button->ResizeToContent();
			hullInfo.button->OnButtonTrigger.Connect([&, hullId = m_hullInfo.size() - 1](const Ndk::ButtonWidget* button)
			{
				OnHullSwitch(hullId);
			});

			if (hullInfo.button->GetSize().x < SpaceshipSize)
				hullInfo.button->Resize({ SpaceshipSize, hullInfo.button->GetSize().y });

			// Entity
			hullInfo.hullEntity = stateData.world3D->CreateEntity();
			auto& hullGfx = hullInfo.hullEntity->AddComponent<Ndk::GraphicsComponent>();
			auto& hullNode = hullInfo.hullEntity->AddComponent<Ndk::NodeComponent>();
			hullNode.SetParent(stateData.camera3D);
			hullNode.SetPosition(Nz::Vector3f::Forward() * 2.f);

			auto& entityGfx = hullInfo.hullEntity->GetComponent<Ndk::GraphicsComponent>();

			entityGfx.Clear();

			// Model
			hullInfo.hullModel = Nz::ModelManager::Get(assetsFolder + '/' + hullInfo.hullPath);
			if (!hullInfo.hullModel)
			{
				std::cerr << "Failed to load model for " << hullData.name << std::endl;
				continue;
			}

			float boundingRadius = hullInfo.hullModel->GetBoundingVolume().obb.localBox.GetRadius();
			Nz::Matrix4f transformMatrix = Nz::Matrix4f::Scale(Nz::Vector3f::Unit() / boundingRadius);

			entityGfx.Attach(hullInfo.hullModel, transformMatrix);
		}

		LayoutWidgets();
	}
}
