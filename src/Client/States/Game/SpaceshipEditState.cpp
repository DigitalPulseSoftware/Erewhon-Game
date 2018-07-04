// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/SpaceshipEditState.hpp>
#include <Nazara/Core/String.hpp>
#include <Nazara/Lua/LuaInstance.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/LightComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/StateMachine.hpp>
#include <cassert>

namespace ewn
{
	void SpaceshipEditState::Enter(Ndk::StateMachine& fsm)
	{
		AbstractState::Enter(fsm);

		StateData& stateData = GetStateData();

		const ConfigFile& config = stateData.app->GetConfig();

		m_deleteConfirmation = false;
		m_labelDisappearanceAccumulator = 0.f;

		m_backButton = CreateWidget<Ndk::ButtonWidget>();
		m_backButton->SetPadding(15.f, 15.f, 15.f, 15.f);
		m_backButton->UpdateText(Nz::SimpleTextDrawer::Draw("Back", 24));
		m_backButton->ResizeToContent();
		m_backButton->OnButtonTrigger.Connect([&](const Ndk::ButtonWidget* /*button*/)
		{
			OnBackPressed();
		});

		m_deleteButton = CreateWidget<Ndk::ButtonWidget>();
		m_deleteButton->SetPadding(15.f, 15.f, 15.f, 15.f);
		m_deleteButton->UpdateText(Nz::SimpleTextDrawer::Draw("Delete spaceship", 24));
		m_deleteButton->ResizeToContent();
		m_deleteButton->OnButtonTrigger.Connect([&](const Ndk::ButtonWidget* /*button*/)
		{
			OnDeletePressed();
		});

		m_createUpdateButton = CreateWidget<Ndk::ButtonWidget>();
		m_createUpdateButton->SetPadding(15.f, 15.f, 15.f, 15.f);

		m_nameLabel = CreateWidget<Ndk::LabelWidget>();
		m_nameLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Spaceship name:", 24));
		m_nameLabel->ResizeToContent();

		m_nameTextArea = CreateWidget<Ndk::TextAreaWidget>();
		m_nameTextArea->SetContentSize({ 160.f, 30.f });
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

		ConnectSignal(stateData.server->OnCreateSpaceshipFailure, this, &SpaceshipEditState::OnCreateSpaceshipFailure);
		ConnectSignal(stateData.server->OnCreateSpaceshipSuccess, this, &SpaceshipEditState::OnCreateSpaceshipSuccess);
		ConnectSignal(stateData.server->OnDeleteSpaceshipFailure, this, &SpaceshipEditState::OnDeleteSpaceshipFailure);
		ConnectSignal(stateData.server->OnDeleteSpaceshipSuccess, this, &SpaceshipEditState::OnDeleteSpaceshipSuccess);
		ConnectSignal(stateData.server->OnModuleList,             this, &SpaceshipEditState::OnModuleList);
		ConnectSignal(stateData.server->OnSpaceshipInfo,          this, &SpaceshipEditState::OnSpaceshipInfo);
		ConnectSignal(stateData.server->OnUpdateSpaceshipFailure, this, &SpaceshipEditState::OnUpdateSpaceshipFailure);
		ConnectSignal(stateData.server->OnUpdateSpaceshipSuccess, this, &SpaceshipEditState::OnUpdateSpaceshipSuccess);

		QueryModuleList();

		if (!IsInEditMode())
		{
			UpdateSpaceshipModel(m_spaceshipModelPath);

			// Create mode
			SetupForCreate();
		}
		else
		{
			// Update mode
			SetupForUpdate();
			QuerySpaceshipInfo();
		}
	}

	void SpaceshipEditState::Leave(Ndk::StateMachine& fsm)
	{
		AbstractState::Leave(fsm);

		m_light.Reset();
		m_spaceship.Reset();
	}

	bool SpaceshipEditState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		StateData& stateData = GetStateData();

		m_labelDisappearanceAccumulator -= elapsedTime;
		if (m_labelDisappearanceAccumulator < 0.f)
			m_statusLabel->Show(false);

		m_spaceship->GetComponent<Ndk::NodeComponent>().Rotate(Nz::EulerAnglesf(0.f, 30.f * elapsedTime, 0.f));

		return true;
	}

	void SpaceshipEditState::LayoutWidgets()
	{
		Nz::Vector2f canvasSize = GetStateData().canvas->GetSize();

		m_backButton->SetPosition(20.f, canvasSize.y - m_backButton->GetSize().y - 20.f);
		m_deleteButton->SetPosition(canvasSize.x - m_deleteButton->GetSize().x - 20.f, canvasSize.y - m_deleteButton->GetSize().y - 20.f);

		// Central
		m_statusLabel->CenterHorizontal();
		m_statusLabel->SetPosition(m_statusLabel->GetPosition().x, canvasSize.y * 0.1f);

		m_titleLabel->CenterHorizontal();
		m_titleLabel->SetPosition(m_titleLabel->GetPosition().x, canvasSize.y * 0.8f - m_titleLabel->GetSize().y / 2.f);

		float totalNameWidth = m_nameLabel->GetSize().x + 5.f + m_nameTextArea->GetSize().x;
		m_nameLabel->SetPosition(canvasSize.x / 2.f - totalNameWidth / 2.f, canvasSize.y * 0.8f - m_titleLabel->GetSize().y / 2.f);
		m_nameTextArea->SetPosition(canvasSize.x / 2.f - totalNameWidth / 2.f + m_nameLabel->GetSize().x, canvasSize.y * 0.8f - m_titleLabel->GetSize().y / 2.f);

		m_createUpdateButton->CenterHorizontal();
		m_createUpdateButton->SetPosition(m_createUpdateButton->GetPosition().x, m_nameTextArea->GetPosition().y + m_nameTextArea->GetSize().y + 20.f);

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

	void SpaceshipEditState::QueryModuleList()
	{
		GetStateData().server->SendPacket(Packets::QueryModuleList{});
	}

	void SpaceshipEditState::QuerySpaceshipInfo()
	{
		m_titleLabel->Show(false);

		UpdateStatus("Loading " + m_spaceshipName + "...");

		Packets::QuerySpaceshipInfo packet;
		packet.info = SpaceshipQueryInfo::HullModelPath | SpaceshipQueryInfo::Modules;
		packet.spaceshipName = m_spaceshipName;

		GetStateData().server->SendPacket(std::move(packet));
	}

	void SpaceshipEditState::SetupForCreate()
	{
		StateData& stateData = GetStateData();

		m_createUpdateButton->UpdateText(Nz::SimpleTextDrawer::Draw("Create", 24));
		m_createUpdateButton->ResizeToContent();

		m_createUpdateButton->OnButtonTrigger.Clear();
		m_createUpdateButton->OnButtonTrigger.Connect([&](const Ndk::ButtonWidget* /*button*/)
		{
			OnCreatePressed();
		});

		m_deleteButton->Show(false);

		m_spaceshipName.clear();

		LayoutWidgets();
	}

	void SpaceshipEditState::SetupForUpdate()
	{
		StateData& stateData = GetStateData();

		m_createUpdateButton->UpdateText(Nz::SimpleTextDrawer::Draw("Update", 24));
		m_createUpdateButton->ResizeToContent();

		m_createUpdateButton->OnButtonTrigger.Clear();
		m_createUpdateButton->OnButtonTrigger.Connect([&](const Ndk::ButtonWidget* /*button*/)
		{
			OnUpdatePressed();
		});

		m_deleteConfirmation = false;
		m_deleteButton->Show(true);
		m_deleteButton->UpdateText(Nz::SimpleTextDrawer::Draw("Delete spaceship", 24));
		m_deleteButton->ResizeToContent();

		m_nameTextArea->SetText(m_spaceshipName);

		LayoutWidgets();
	}

	void SpaceshipEditState::UpdateSpaceshipModel(const std::string& hullModelPath)
	{
		StateData& stateData = GetStateData();

		const std::string& assetsFolder = stateData.app->GetConfig().GetStringOption("AssetsFolder");

		Nz::ModelRef spaceshipModel = Nz::ModelManager::Get(assetsFolder + '/' + hullModelPath);
		if (!spaceshipModel)
		{
			UpdateStatus("Failed to load model", Nz::Color::Red);
			return;
		}

		float boundingRadius = spaceshipModel->GetBoundingVolume().obb.localBox.GetRadius();
		Nz::Matrix4f transformMatrix = Nz::Matrix4f::Scale(Nz::Vector3f::Unit() / boundingRadius);

		auto& entityGfx = m_spaceship->GetComponent<Ndk::GraphicsComponent>();

		entityGfx.Clear();
		entityGfx.Attach(spaceshipModel, transformMatrix);
	}

	void SpaceshipEditState::UpdateStatus(const Nz::String& status, const Nz::Color& color)
	{
		m_statusLabel->Show(true);
		m_statusLabel->UpdateText(Nz::SimpleTextDrawer::Draw(status, 24, 0U, color));
		m_statusLabel->ResizeToContent();

		m_labelDisappearanceAccumulator = status.GetLength() / 10.f;

		LayoutWidgets();
	}

	void SpaceshipEditState::OnBackPressed()
	{
		GetStateData().fsm->ChangeState(m_previousState);
	}

	void SpaceshipEditState::OnCreatePressed()
	{
		assert(!IsInEditMode());
		assert(m_spaceshipHullId != 0xFFFFFFFF);

		Nz::String spaceshipName = m_nameTextArea->GetText();
		if (spaceshipName.IsEmpty())
		{
			UpdateStatus("Missing spaceship name", Nz::Color::Red);
			return;
		}

		if (m_spaceshipCode.IsEmpty())
		{
			UpdateStatus("Please load a script file", Nz::Color::Red);
			return;
		}

		Packets::CreateSpaceship createSpaceship;
		createSpaceship.hullId = m_spaceshipHullId;
		createSpaceship.spaceshipName = spaceshipName.ToStdString();
		createSpaceship.spaceshipCode = m_spaceshipCode.ToStdString();
		m_spaceshipCode.Clear(true);

		for (auto& buttonData : m_moduleButtons)
		{
			auto& modifiedModule = createSpaceship.modules.emplace_back();
			modifiedModule.moduleId = static_cast<Nz::UInt16>(buttonData.availableChoices[buttonData.currentChoice].moduleId);
			modifiedModule.type = buttonData.moduleType;

			buttonData.originalChoice = buttonData.currentChoice;
		}

		GetStateData().server->SendPacket(createSpaceship);
	}

	void SpaceshipEditState::OnDeletePressed()
	{
		assert(IsInEditMode());

		if (!m_deleteConfirmation)
		{
			m_deleteButton->UpdateText(Nz::SimpleTextDrawer::Draw("Delete spaceship\n(confirm)", 24));
			m_deleteButton->ResizeToContent();

			m_deleteConfirmation = true;

			LayoutWidgets();
			return;
		}

		m_deleteConfirmation = false;

		Packets::DeleteSpaceship deleteSpaceship;
		deleteSpaceship.spaceshipName = m_spaceshipName;

		GetStateData().server->SendPacket(deleteSpaceship);
	}

	void SpaceshipEditState::OnLoadCodePressed()
	{
		Nz::String fileName = m_codeFilenameTextArea->GetText();
		if (fileName.IsEmpty())
		{
			UpdateStatus("Invalid filename", Nz::Color::Red);
			return;
		}

		// Load file content
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

		// Check Lua syntax
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

		moduleInfo.button->UpdateText(Nz::SimpleTextDrawer::Draw(std::string(EnumToString(moduleInfo.moduleType)) + " module\n(" + moduleInfo.availableChoices[moduleInfo.currentChoice].moduleName + ')', 18));
		moduleInfo.button->ResizeToContent();

		LayoutWidgets();
	}

	void SpaceshipEditState::OnUpdatePressed()
	{
		assert(IsInEditMode());

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
				modifiedModule.moduleName = buttonData.availableChoices[buttonData.currentChoice].moduleName;
				modifiedModule.oldModuleName = buttonData.availableChoices[buttonData.originalChoice].moduleName;
				modifiedModule.type = buttonData.moduleType;

				buttonData.originalChoice = buttonData.currentChoice;
			}
		}

		//TODO: Check for any change

		GetStateData().server->SendPacket(updateSpaceship);
	}

	void SpaceshipEditState::OnCreateSpaceshipFailure(ServerConnection* server, const Packets::CreateSpaceshipFailure& createPacket)
	{
		std::string reason;
		switch (createPacket.reason)
		{
			case CreateSpaceshipFailureReason::AlreadyExists:
				reason = "spaceship name is already taken";
				break;

			case CreateSpaceshipFailureReason::ServerError:
				reason = "server error, please try again later";
				break;

			default:
				reason = "<packet error>";
				break;
		}

		UpdateStatus("Failed to create spaceship: " + reason, Nz::Color::Red);
	}

	void SpaceshipEditState::OnCreateSpaceshipSuccess(ServerConnection* server, const Packets::CreateSpaceshipSuccess& createPacket)
	{
		UpdateStatus("Spaceship successfully created", Nz::Color::Green);

		m_spaceshipName = m_nameTextArea->GetText().ToStdString();
		m_isInEditMode = true;

		SetupForUpdate();
	}

	void SpaceshipEditState::OnDeleteSpaceshipFailure(ServerConnection* server, const Packets::DeleteSpaceshipFailure& deletePacket)
	{
		std::string reason;
		switch (deletePacket.reason)
		{
			case DeleteSpaceshipFailureReason::MustHaveAtLeastOne:
				reason = "you cannot delete your last spaceship";
				break;

			case DeleteSpaceshipFailureReason::NotFound:
				reason = "spaceship not found";
				break;

			case DeleteSpaceshipFailureReason::ServerError:
				reason = "server error, please try again later";
				break;

			default:
				reason = "<packet error>";
				break;
		}

		UpdateStatus("Failed to delete spaceship: " + reason, Nz::Color::Red);
	}

	void SpaceshipEditState::OnDeleteSpaceshipSuccess(ServerConnection* server, const Packets::DeleteSpaceshipSuccess& deletePacket)
	{
		UpdateStatus("Spaceship successfully deleted", Nz::Color::Green);

		m_isInEditMode = false;
		SetupForCreate();
	}

	void SpaceshipEditState::OnModuleList(ServerConnection* server, const Packets::ModuleList& moduleList)
	{
		// Module buttons
		m_moduleButtons.clear();
		for (const auto& moduleData : moduleList.modules)
		{
			ModuleInfo& moduleInfo = m_moduleButtons.emplace_back();
			moduleInfo.currentChoice = 0;
			moduleInfo.moduleType = moduleData.type;
			moduleInfo.originalChoice = moduleInfo.currentChoice;

			for (const auto& packetChoice : moduleData.availableModules)
			{
				auto& choice = moduleInfo.availableChoices.emplace_back();
				choice.moduleId = packetChoice.moduleId;
				choice.moduleName = packetChoice.moduleName;
			}

			moduleInfo.button = CreateWidget<Ndk::ButtonWidget>();
			moduleInfo.button->SetPadding(15.f, 15.f, 15.f, 15.f);
			moduleInfo.button->UpdateText(Nz::SimpleTextDrawer::Draw(std::string(EnumToString(moduleData.type)) + " module\n(" + moduleInfo.availableChoices[moduleInfo.currentChoice].moduleName + ')', 18));
			moduleInfo.button->ResizeToContent();
			moduleInfo.button->OnButtonTrigger.Connect([&, moduleId = m_moduleButtons.size() - 1](const Ndk::ButtonWidget* button)
			{
				OnModuleSwitch(moduleId);
			});
		}

		LayoutWidgets();
	}

	void SpaceshipEditState::OnSpaceshipInfo(ServerConnection* server, const Packets::SpaceshipInfo& infoPacket)
	{
		if (infoPacket.hullModelPath.empty())
		{
			UpdateStatus("Failed to load spaceship", Nz::Color::Red);
			return;
		}

		m_spaceshipHullId = infoPacket.hullId;

		UpdateSpaceshipModel(infoPacket.hullModelPath);

		m_statusLabel->Show(false);
		//m_titleLabel->Show(true);

		m_titleLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Spaceship " + m_spaceshipName + ":", 24));
		m_titleLabel->ResizeToContent();

		// Module buttons
		for (const auto& moduleData : infoPacket.modules)
		{
			auto buttonIt = std::find_if(m_moduleButtons.begin(), m_moduleButtons.end(), [type = moduleData.type](const ModuleInfo& moduleInfo)
			{
				return moduleInfo.moduleType == type;
			});
			assert(buttonIt != m_moduleButtons.end());

			ModuleInfo& moduleInfo = *buttonIt;
			auto choiceIt = std::find_if(moduleInfo.availableChoices.begin(), moduleInfo.availableChoices.end(), [id = moduleData.currentModule](const ModuleInfo::ModuleChoice& choice)
			{
				return choice.moduleId == id;
			});
			assert(choiceIt != moduleInfo.availableChoices.end());

			moduleInfo.currentChoice = std::distance(moduleInfo.availableChoices.begin(), choiceIt);
			moduleInfo.originalChoice = moduleInfo.currentChoice;

			moduleInfo.button->UpdateText(Nz::SimpleTextDrawer::Draw(std::string(EnumToString(moduleData.type)) + " module\n(" + moduleInfo.availableChoices[moduleInfo.currentChoice].moduleName + ')', 18));
			moduleInfo.button->ResizeToContent();
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
}
