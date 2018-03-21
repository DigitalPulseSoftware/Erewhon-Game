// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/OptionState.hpp>
#include <Nazara/Core/File.hpp>
#include <NDK/StateMachine.hpp>
#include <Client/ClientApplication.hpp>
#include <Client/States/LoginState.hpp>
#include <iostream>

namespace ewn
{
	void OptionState::Enter(Ndk::StateMachine& /*fsm*/)
	{
		m_isReturningBack = false;

		m_fullscreenCheckbox = m_stateData.canvas->Add<Ndk::CheckboxWidget>();
		m_fullscreenCheckbox->UpdateText(Nz::SimpleTextDrawer::Draw("Fullscreen mode (need restart)", 24));
		m_fullscreenCheckbox->ResizeToContent();

		m_statusLabel = m_stateData.canvas->Add<Ndk::LabelWidget>();
		m_statusLabel->Show(false);

		m_vsyncCheckbox = m_stateData.canvas->Add<Ndk::CheckboxWidget>();
		m_vsyncCheckbox->UpdateText(Nz::SimpleTextDrawer::Draw("Vertical sync", 24));
		m_vsyncCheckbox->ResizeToContent();

		m_applyButton = m_stateData.canvas->Add<Ndk::ButtonWidget>();
		m_applyButton->UpdateText(Nz::SimpleTextDrawer::Draw("Apply", 24));
		m_applyButton->ResizeToContent();
		m_applyButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnApplyPressed();
		});

		m_backButton = m_stateData.canvas->Add<Ndk::ButtonWidget>();
		m_backButton->UpdateText(Nz::SimpleTextDrawer::Draw("Back", 24));
		m_backButton->ResizeToContent();
		m_backButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnBackPressed();
		});

		// Set both connection and register button of the same width
		constexpr float buttonPadding = 25.f;
		float regConnWidth = std::max(m_applyButton->GetSize().x, m_backButton->GetSize().x) + buttonPadding;
		m_applyButton->SetSize({ regConnWidth, m_applyButton->GetSize().y + buttonPadding });
		m_backButton->SetSize({ regConnWidth, m_backButton->GetSize().y + buttonPadding });

		LayoutWidgets();
		m_onTargetChangeSizeSlot.Connect(m_stateData.window->OnRenderTargetSizeChange, [this](const Nz::RenderTarget*) { LayoutWidgets(); });

		LoadOptions();
	}

	void OptionState::Leave(Ndk::StateMachine& /*fsm*/)
	{
		m_applyButton->Destroy();
		m_backButton->Destroy();
		m_fullscreenCheckbox->Destroy();
		m_statusLabel->Destroy();
		m_vsyncCheckbox->Destroy();
	}

	bool OptionState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (m_isReturningBack)
			fsm.ChangeState(std::make_shared<LoginState>(m_stateData));

		return true;
	}

	void OptionState::LayoutWidgets()
	{
		Nz::Vector2f center = m_stateData.canvas->GetSize() / 2.f;

		constexpr float padding = 10.f;

		std::array<Ndk::BaseWidget*, 2> widgets = {
			m_fullscreenCheckbox,
			m_vsyncCheckbox
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

		m_fullscreenCheckbox->SetPosition({ center.x - maxWidth / 2.f , cursor.y, 0.f });
		cursor.y += m_fullscreenCheckbox->GetSize().y + padding;

		m_vsyncCheckbox->SetPosition({ center.x - maxWidth / 2.f , cursor.y, 0.f });
		cursor.y += m_vsyncCheckbox->GetSize().y + padding;

		
		cursor.y += padding;

		constexpr float extraPadding = 10.f;
		float buttonWidth = std::max(m_applyButton->GetSize().x, m_backButton->GetSize().x);
		m_backButton->SetPosition({ center.x - buttonWidth - extraPadding, cursor.y, 0.f });
		m_applyButton->SetPosition({ center.x + extraPadding, cursor.y, 0.f });
	}

	void OptionState::OnApplyPressed()
	{
		ApplyOptions();
		SaveOptions();

		UpdateStatus("Options have been saved successfully, you may have to restart the game to apply them", Nz::Color::Green);
	}

	void OptionState::OnBackPressed()
	{
		m_isReturningBack = true;
	}

	void OptionState::ApplyOptions()
	{
		ConfigFile& configFile = m_stateData.app->GetConfig();

		configFile.SetBoolOption("Options.Fullscreen", m_fullscreenCheckbox->GetState() == Ndk::CheckboxState_Checked);
		configFile.SetBoolOption("Options.VerticalSync", m_vsyncCheckbox->GetState() == Ndk::CheckboxState_Checked);

		m_stateData.window->EnableVerticalSync(m_vsyncCheckbox->GetState() == Ndk::CheckboxState_Checked);
	}

	void OptionState::LoadOptions()
	{
		const ConfigFile& configFile = m_stateData.app->GetConfig();

		m_fullscreenCheckbox->SetState((configFile.GetBoolOption("Options.Fullscreen")) ? Ndk::CheckboxState_Checked : Ndk::CheckboxState_Unchecked);
		m_vsyncCheckbox->SetState((configFile.GetBoolOption("Options.VerticalSync")) ? Ndk::CheckboxState_Checked : Ndk::CheckboxState_Unchecked);
	}

	void OptionState::SaveOptions()
	{
		Nz::File optionFile("coptions.lua", Nz::OpenMode_Truncate | Nz::OpenMode_WriteOnly);
		if (!optionFile.IsOpen())
		{
			std::cerr << "Failed to open option file" << std::endl;
			return;
		}

		const ConfigFile& configFile = m_stateData.app->GetConfig();

		std::array<std::string, 2> boolOptions =
		{
			"Fullscreen",
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

	void OptionState::UpdateStatus(const Nz::String& status, const Nz::Color& color)
	{
		m_statusLabel->UpdateText(Nz::SimpleTextDrawer::Draw(status, 24, 0L, color));
		m_statusLabel->ResizeToContent();
		m_statusLabel->CenterHorizontal();
		m_statusLabel->Show(true);
	}
}
