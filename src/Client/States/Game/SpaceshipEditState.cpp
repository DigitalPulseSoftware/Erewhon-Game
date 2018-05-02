// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/SpaceshipEditState.hpp>
#include <Nazara/Core/String.hpp>
#include <Nazara/Lua/LuaInstance.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/LightComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/StateMachine.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/ConnectionLostState.hpp>
#include <Client/States/Game/MainMenuState.hpp>
#include <Client/States/Game/TimeSyncState.hpp>
#include <cassert>

namespace ewn
{
	void SpaceshipEditState::Enter(Ndk::StateMachine& /*fsm*/)
	{
		StateData& stateData = GetStateData();

		const ConfigFile& config = stateData.app->GetConfig();

		m_labelDisappearanceAccumulator = 0.f;

		m_backButton = CreateWidget<Ndk::ButtonWidget>();
		m_backButton->SetPadding(15.f, 15.f, 15.f, 15.f);
		m_backButton->UpdateText(Nz::SimpleTextDrawer::Draw("Back", 24));
		m_backButton->ResizeToContent();
		m_backButton->OnButtonTrigger.Connect([&](const Ndk::ButtonWidget* /*button*/)
		{
			OnBackPressed();
		});

		m_updateButton = CreateWidget<Ndk::ButtonWidget>();
		m_updateButton->SetPadding(15.f, 15.f, 15.f, 15.f);
		m_updateButton->UpdateText(Nz::SimpleTextDrawer::Draw("Update", 24));
		m_updateButton->ResizeToContent();
		m_updateButton->OnButtonTrigger.Connect([&](const Ndk::ButtonWidget* /*button*/)
		{
			OnUpdatePressed();
		});

		m_nameLabel = CreateWidget<Ndk::LabelWidget>();
		m_nameLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Spaceship name:", 24));
		m_nameLabel->ResizeToContent();

		m_nameTextArea = CreateWidget<Ndk::TextAreaWidget>();
		m_nameTextArea->SetContentSize({ 160.f, 30.f });
		m_nameTextArea->SetText(m_spaceshipName);
		m_nameTextArea->EnableBackground(true);
		m_nameTextArea->SetBackgroundColor(Nz::Color::White);
		m_nameTextArea->SetTextColor(Nz::Color::Black);

		m_codeLoadButton = CreateWidget<Ndk::ButtonWidget>();
		m_codeLoadButton->SetPadding(15.f, 15.f, 15.f, 15.f);
		m_codeLoadButton->UpdateText(Nz::SimpleTextDrawer::Draw("Load code", 24));
		m_codeLoadButton->ResizeToContent();
		m_codeLoadButton->OnButtonTrigger.Connect([&](const Ndk::ButtonWidget* /*button*/)
		{
			OnLoadCodePressed();
		});

		m_codeFilenameLabel = CreateWidget<Ndk::LabelWidget>();
		m_codeFilenameLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Code input (file name):", 24));
		m_codeFilenameLabel->ResizeToContent();

		m_codeFilenameTextArea = CreateWidget<Ndk::TextAreaWidget>();
		m_codeFilenameTextArea->SetContentSize({ 300.f, 30.f });
		m_codeFilenameTextArea->SetText(config.GetStringOption("ServerScript.Filename"));
		m_codeFilenameTextArea->EnableBackground(true);
		m_codeFilenameTextArea->SetBackgroundColor(Nz::Color::White);
		m_codeFilenameTextArea->SetTextColor(Nz::Color::Black);

		m_statusLabel = CreateWidget<Ndk::LabelWidget>();
		m_titleLabel = CreateWidget<Ndk::LabelWidget>();

		m_light = stateData.world3D->CreateEntity();
		m_light->AddComponent<Ndk::LightComponent>(Nz::LightType_Spot);
		auto& lightNode = m_light->AddComponent<Ndk::NodeComponent>();
		lightNode.SetParent(stateData.camera3D);

		m_spaceship = stateData.world3D->CreateEntity();
		m_spaceship->AddComponent<Ndk::GraphicsComponent>();
		auto& spaceshipNode = m_spaceship->AddComponent<Ndk::NodeComponent>();
		spaceshipNode.SetParent(stateData.camera3D);
		spaceshipNode.SetPosition(Nz::Vector3f::Forward() * 2.f);

		LayoutWidgets();
		m_onTargetChangeSizeSlot.Connect(stateData.window->OnRenderTargetSizeChange, [this](const Nz::RenderTarget*) { LayoutWidgets(); });

		m_onUpdateSpaceshipFailureSlot.Connect(stateData.server->OnUpdateSpaceshipFailure, this, &SpaceshipEditState::OnUpdateSpaceshipFailure);
		m_onUpdateSpaceshipSuccessSlot.Connect(stateData.server->OnUpdateSpaceshipSuccess, this, &SpaceshipEditState::OnUpdateSpaceshipSuccess);
		m_onSpaceshipInfoSlot.Connect(stateData.server->OnSpaceshipInfo, this, &SpaceshipEditState::OnSpaceshipInfo);

		QuerySpaceshipInfo();
	}

	void SpaceshipEditState::Leave(Ndk::StateMachine& fsm)
	{
		AbstractState::Leave(fsm);

		m_light.Reset();
		m_spaceship.Reset();

		m_onSpaceshipInfoSlot.Disconnect();
		m_onTargetChangeSizeSlot.Disconnect();
	}

	bool SpaceshipEditState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		StateData& stateData = GetStateData();

		if (!stateData.server->IsConnected())
		{
			fsm.ChangeState(std::make_shared<ConnectionLostState>(stateData));
			return false;
		}

		m_labelDisappearanceAccumulator -= elapsedTime;
		if (m_labelDisappearanceAccumulator < 0.f)
			m_statusLabel->Show(false);

		if (m_nextState)
			fsm.ChangeState(m_nextState);

		m_spaceship->GetComponent<Ndk::NodeComponent>().Rotate(Nz::EulerAnglesf(0.f, 30.f * elapsedTime, 0.f));

		return true;
	}

	void SpaceshipEditState::LayoutWidgets()
	{
		Nz::Vector2f canvasSize = GetStateData().canvas->GetSize();

		// Central
		m_backButton->SetPosition(20.f, canvasSize.y - m_backButton->GetSize().y - 20.f);

		m_statusLabel->CenterHorizontal();
		m_statusLabel->SetPosition(m_statusLabel->GetPosition().x, canvasSize.y * 0.1f);

		m_titleLabel->CenterHorizontal();
		m_titleLabel->SetPosition(m_titleLabel->GetPosition().x, canvasSize.y * 0.8f - m_titleLabel->GetSize().y / 2.f);

		float totalNameWidth = m_nameLabel->GetSize().x + 5.f + m_nameTextArea->GetSize().x;
		m_nameLabel->SetPosition(canvasSize.x / 2.f - totalNameWidth / 2.f, canvasSize.y * 0.8f - m_titleLabel->GetSize().y / 2.f);
		m_nameTextArea->SetPosition(canvasSize.x / 2.f - totalNameWidth / 2.f + m_nameLabel->GetSize().x, canvasSize.y * 0.8f - m_titleLabel->GetSize().y / 2.f);

		m_updateButton->CenterHorizontal();
		m_updateButton->SetPosition(m_updateButton->GetPosition().x, m_nameTextArea->GetPosition().y + m_nameTextArea->GetSize().y + 20.f);

		Nz::Vector2f cursor;

		// Left menu (modules)
		static constexpr float modulePadding = 10.f;

		cursor = Nz::Vector2f(canvasSize.x * 0.15f, canvasSize.y * 0.2f);
		for (const auto& buttonData : m_moduleButtons)
		{
			buttonData.button->SetPosition({ cursor.x - buttonData.button->GetSize().x / 2.f, cursor.y, 0.f });
			cursor.y += buttonData.button->GetSize().y + modulePadding;
		}

		// Right menu (code)
		static constexpr float padding = 10.f;

		cursor = Nz::Vector2f(canvasSize.x * 0.85f, canvasSize.y * 0.2f);

		m_codeFilenameLabel->SetPosition(cursor.x - m_codeFilenameLabel->GetSize().x / 2.f, cursor.y);
		cursor.y += m_codeFilenameLabel->GetSize().y + padding;

		m_codeFilenameTextArea->SetPosition(cursor.x - m_codeFilenameTextArea->GetSize().x / 2.f, cursor.y);
		cursor.y += m_codeFilenameTextArea->GetSize().y + padding;

		m_codeLoadButton->SetPosition(cursor.x - m_codeLoadButton->GetSize().x / 2.f, cursor.y);
		cursor.y += m_codeLoadButton->GetSize().y + padding * 3.f;
	}

	void SpaceshipEditState::OnBackPressed()
	{
		m_nextState = m_previousState;
	}

	void SpaceshipEditState::OnLoadCodePressed()
	{
		Nz::String fileName = m_codeFilenameTextArea->GetText();
		if (fileName.IsEmpty())
		{
			UpdateStatus("Invalid filename", Nz::Color::Red);
			return;
		}

		Nz::File file(fileName, Nz::OpenMode_ReadOnly | Nz::OpenMode_Text);
		if (!file.IsOpen())
		{
			UpdateStatus("Failed to open " + fileName, Nz::Color::Red);
			return;
		}

		Nz::String content;
		content.Reserve(file.GetSize());

		while (!file.EndOfFile())
		{
			content += file.ReadLine();
			content += '\n';
		}

		Nz::LuaInstance lua;
		if (!lua.Load(content))
		{
			std::cerr << "Parsing error in " << fileName << ": " << lua.GetLastError() << std::endl;
			UpdateStatus("Parsing error: " + lua.GetLastError(), Nz::Color::Red);
			return;
		}

		m_spaceshipCode = content;
		UpdateStatus(fileName + " successfully loaded");
	}

	void SpaceshipEditState::OnModuleSwitch(std::size_t moduleId)
	{
		ModuleInfo& moduleInfo = m_moduleButtons[moduleId];
		moduleInfo.currentChoice++;
		if (moduleInfo.currentChoice >= moduleInfo.availableChoices.size())
			moduleInfo.currentChoice = 0;

		moduleInfo.button->UpdateText(Nz::SimpleTextDrawer::Draw(std::string(EnumToString(moduleInfo.moduleType)) + " module\n(" + moduleInfo.availableChoices[moduleInfo.currentChoice] + ')', 18));
		moduleInfo.button->ResizeToContent();

		LayoutWidgets();
	}

	void SpaceshipEditState::OnSpaceshipInfo(ServerConnection* server, const Packets::SpaceshipInfo& listPacket)
	{
		const std::string& assetsFolder = server->GetApp().GetConfig().GetStringOption("AssetsFolder");

		m_statusLabel->Show(false);
		//m_titleLabel->Show(true);

		m_titleLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Spaceship " + m_spaceshipName + ":", 24));
		m_titleLabel->ResizeToContent();

		LayoutWidgets();

		auto& entityGfx = m_spaceship->GetComponent<Ndk::GraphicsComponent>();

		entityGfx.Clear();

		Nz::ModelParameters modelParams;
		modelParams.mesh.center = true;
		modelParams.mesh.texCoordScale.Set(1.f, -1.f);

		m_spaceshipModel = Nz::Model::New();
		if (!m_spaceshipModel->LoadFromFile(assetsFolder + '/' + listPacket.hullModelPath, modelParams))
		{
			UpdateStatus("Failed to load model", Nz::Color::Red);
			return;
		}

		float boundingRadius = m_spaceshipModel->GetBoundingVolume().obb.localBox.GetRadius();
		Nz::Matrix4f transformMatrix = Nz::Matrix4f::Scale(Nz::Vector3f::Unit() / boundingRadius);

		entityGfx.Attach(m_spaceshipModel, transformMatrix);

		// Module buttons
		m_moduleButtons.clear();
		for (const auto& moduleData : listPacket.modules)
		{
			ModuleInfo& moduleInfo = m_moduleButtons.emplace_back();
			moduleInfo.availableChoices = moduleData.availableModules;
			moduleInfo.currentChoice    = moduleData.currentModule;
			moduleInfo.moduleType       = moduleData.type;
			moduleInfo.originalChoice   = moduleInfo.currentChoice;

			moduleInfo.button = CreateWidget<Ndk::ButtonWidget>();
			moduleInfo.button->SetPadding(15.f, 15.f, 15.f, 15.f);
			moduleInfo.button->UpdateText(Nz::SimpleTextDrawer::Draw(std::string(EnumToString(moduleData.type)) + " module\n(" + moduleInfo.availableChoices[moduleInfo.currentChoice] + ')', 18));
			moduleInfo.button->ResizeToContent();
			moduleInfo.button->OnButtonTrigger.Connect([&, moduleId = m_moduleButtons.size() - 1](const Ndk::ButtonWidget* button)
			{
				OnModuleSwitch(moduleId);
			});
		}

		LayoutWidgets();
	}

	void SpaceshipEditState::OnUpdateSpaceshipFailure(ServerConnection* server, const Packets::UpdateSpaceshipFailure& updatePacket)
	{
		std::string reason;
		switch (updatePacket.reason)
		{
			case UpdateSpaceshipFailureReason::NotFound:
				reason = "spaceship not found";
				break;

			case UpdateSpaceshipFailureReason::ServerError:
				reason = "server error, please try again later";
				break;

			default:
				reason = "<packet error>";
				break;
		}

		UpdateStatus("Failed to update spaceship: " + reason, Nz::Color::Red);
	}

	void SpaceshipEditState::OnUpdateSpaceshipSuccess(ServerConnection* server, const Packets::UpdateSpaceshipSuccess& updatePacket)
	{
		UpdateStatus("Spaceship successfully updated", Nz::Color::Green);

		m_spaceshipName = m_nameTextArea->GetText().ToStdString();
	}

	void SpaceshipEditState::OnUpdatePressed()
	{
		Packets::UpdateSpaceship updateSpaceship;
		updateSpaceship.spaceshipName = m_spaceshipName;
		updateSpaceship.newSpaceshipName = m_nameTextArea->GetText().ToStdString();
		if (updateSpaceship.newSpaceshipName == updateSpaceship.spaceshipName)
			updateSpaceship.newSpaceshipName.clear(); //< Don't send new-name

		updateSpaceship.newSpaceshipCode = m_spaceshipCode.ToStdString();
		m_spaceshipCode.Clear(true);

		for (auto& buttonData : m_moduleButtons)
		{
			if (buttonData.originalChoice != buttonData.currentChoice)
			{
				auto& modifiedModule = updateSpaceship.modifiedModules.emplace_back();
				modifiedModule.moduleName = buttonData.availableChoices[buttonData.currentChoice];
				modifiedModule.oldModuleName = buttonData.availableChoices[buttonData.originalChoice];
				modifiedModule.type = buttonData.moduleType;

				buttonData.originalChoice = buttonData.currentChoice;
			}
		}

		//TODO: Check for any change

		GetStateData().server->SendPacket(updateSpaceship);
	}

	void SpaceshipEditState::QuerySpaceshipInfo()
	{
		m_titleLabel->Show(false);

		UpdateStatus("Loading " + m_spaceshipName + "...");

		Packets::QuerySpaceshipInfo packet;
		packet.spaceshipName = m_spaceshipName;

		GetStateData().server->SendPacket(std::move(packet));
	}

	void SpaceshipEditState::UpdateStatus(const Nz::String& status, const Nz::Color& color)
	{
		m_statusLabel->Show(true);
		m_statusLabel->UpdateText(Nz::SimpleTextDrawer::Draw(status, 24, 0U, color));
		m_statusLabel->ResizeToContent();

		m_labelDisappearanceAccumulator = status.GetLength() / 10.f;

		LayoutWidgets();
	}
}
