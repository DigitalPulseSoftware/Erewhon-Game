// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/OptionsState.hpp>
#include <Nazara/Core/File.hpp>
#include <NDK/StateMachine.hpp>
#include <Client/ClientApplication.hpp>
#include <Client/States/LoginState.hpp>
#include <iostream>

namespace ewn
{
	void OptionsState::Enter(Ndk::StateMachine& /*fsm*/)
	{
		StateData& stateData = GetStateData();

		m_isReturningBack = false;

		m_forceIPv4Checkbox = CreateWidget<Ndk::CheckboxWidget>();
		m_forceIPv4Checkbox->UpdateText(Nz::SimpleTextDrawer::Draw("Force IPv4 for new connections", 24));
		m_forceIPv4Checkbox->ResizeToContent();

		m_fullscreenCheckbox = CreateWidget<Ndk::CheckboxWidget>();
		m_fullscreenCheckbox->UpdateText(Nz::SimpleTextDrawer::Draw("Fullscreen mode (need restart)", 24));
		m_fullscreenCheckbox->ResizeToContent();

		m_statusLabel = CreateWidget<Ndk::LabelWidget>();
		m_statusLabel->Show(false);

		m_vsyncCheckbox = CreateWidget<Ndk::CheckboxWidget>();
		m_vsyncCheckbox->UpdateText(Nz::SimpleTextDrawer::Draw("Vertical sync", 24));
		m_vsyncCheckbox->ResizeToContent();

		m_applyButton = CreateWidget<Ndk::ButtonWidget>();
		m_applyButton->UpdateText(Nz::SimpleTextDrawer::Draw("Apply", 24));
		m_applyButton->SetPadding(10.f, 10.f, 10.f, 10.f);
		m_applyButton->ResizeToContent();
		m_applyButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnApplyPressed();
		});

		m_backButton = CreateWidget<Ndk::ButtonWidget>();
		m_backButton->UpdateText(Nz::SimpleTextDrawer::Draw("Back", 24));
		m_backButton->SetPadding(10.f, 10.f, 10.f, 10.f);
		m_backButton->ResizeToContent();
		m_backButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnBackPressed();
		});

		LayoutWidgets();
		m_onTargetChangeSizeSlot.Connect(stateData.window->OnRenderTargetSizeChange, [this](const Nz::RenderTarget*) { LayoutWidgets(); });

		LoadOptions();
	}

	bool OptionsState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (m_isReturningBack)
			fsm.ChangeState(std::move(m_previousState));

		return true;
	}

	void OptionsState::LayoutWidgets()
	{
		StateData& stateData = GetStateData();

		Nz::Vector2f center = stateData.canvas->GetSize() / 2.f;

		constexpr float padding = 10.f;

		std::array<Ndk::BaseWidget*, 3> widgets = {
			m_fullscreenCheckbox,
			m_vsyncCheckbox,
			m_forceIPv4Checkbox
		};

		float maxWidth = 0.f;
		float totalSize = padding * (widgets.size() - 1);
		for (Ndk::BaseWidget* widget : widgets)
		{
			Nz::Vector2f size = widget->GetSize();
			maxWidth = std::max(maxWidth, size.x);
			totalSize += size.y;
		}

		Nz::Vector2f cursor = center;
		cursor.y -= totalSize / 2.f;

		m_statusLabel->SetPosition({ 0.f, cursor.y - m_statusLabel->GetSize().y - padding, 0.f });

		for (Ndk::BaseWidget* widget : widgets)
		{
			widget->SetPosition({ center.x - maxWidth / 2.f , cursor.y, 0.f });
			cursor.y += widget->GetSize().y + padding;
		}
		
		cursor.y += padding;

		constexpr float extraPadding = 10.f;
		float buttonWidth = std::max(m_applyButton->GetSize().x, m_backButton->GetSize().x);
		m_backButton->SetPosition({ center.x - buttonWidth - extraPadding, cursor.y, 0.f });
		m_applyButton->SetPosition({ center.x + extraPadding, cursor.y, 0.f });
	}

	void OptionsState::OnApplyPressed()
	{
		ApplyOptions();
		SaveOptions();

		UpdateStatus("Options have been saved successfully, you may have to restart the game to apply them", Nz::Color::Green);
	}

	void OptionsState::OnBackPressed()
	{
		m_isReturningBack = true;
	}

	void OptionsState::ApplyOptions()
	{
		StateData& stateData = GetStateData();

		ConfigFile& configFile = stateData.app->GetConfig();

		configFile.SetBoolOption("Options.ForceIPv4", m_forceIPv4Checkbox->GetState() == Ndk::CheckboxState_Checked);
		configFile.SetBoolOption("Options.Fullscreen", m_fullscreenCheckbox->GetState() == Ndk::CheckboxState_Checked);
		configFile.SetBoolOption("Options.VerticalSync", m_vsyncCheckbox->GetState() == Ndk::CheckboxState_Checked);

		stateData.window->EnableVerticalSync(m_vsyncCheckbox->GetState() == Ndk::CheckboxState_Checked);
	}

	void OptionsState::LoadOptions()
	{
		StateData& stateData = GetStateData();
		const ConfigFile& configFile = stateData.app->GetConfig();

		m_forceIPv4Checkbox->SetState((configFile.GetBoolOption("Options.ForceIPv4")) ? Ndk::CheckboxState_Checked : Ndk::CheckboxState_Unchecked);
		m_fullscreenCheckbox->SetState((configFile.GetBoolOption("Options.Fullscreen")) ? Ndk::CheckboxState_Checked : Ndk::CheckboxState_Unchecked);
		m_vsyncCheckbox->SetState((configFile.GetBoolOption("Options.VerticalSync")) ? Ndk::CheckboxState_Checked : Ndk::CheckboxState_Unchecked);
	}

	void OptionsState::SaveOptions()
	{
		StateData& stateData = GetStateData();

		Nz::File optionFile("coptions.lua", Nz::OpenMode_Truncate | Nz::OpenMode_WriteOnly);
		if (!optionFile.IsOpen())
		{
			std::cerr << "Failed to open option file" << std::endl;
			return;
		}

		const ConfigFile& configFile = stateData.app->GetConfig();

		std::array<std::string, 3> boolOptions =
		{
			"Fullscreen",
			"ForceIPv4",
			"VerticalSync"
		};

		optionFile.Write("Options = {\n");

		for (const std::string& optionName : boolOptions)
		{
			bool value = configFile.GetBoolOption("Options." + optionName);
			std::string valueAsText = (value) ? "true" : "false";
			optionFile.Write("\t" + optionName + " = " + valueAsText + ",\n");
		}

		optionFile.Write("}\n");
	}

	void OptionsState::UpdateStatus(const Nz::String& status, const Nz::Color& color)
	{
		m_statusLabel->UpdateText(Nz::SimpleTextDrawer::Draw(status, 24, 0L, color));
		m_statusLabel->ResizeToContent();
		m_statusLabel->CenterHorizontal();
		m_statusLabel->Show(true);
	}
}
