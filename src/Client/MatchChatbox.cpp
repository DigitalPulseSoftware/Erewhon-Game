// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/MatchChatbox.hpp>

namespace ewn
{
	static constexpr std::size_t maxChatLines = 15;

	MatchChatbox::MatchChatbox(ServerConnection* server, Nz::RenderWindow& window, Ndk::Canvas* canvas) :
	m_chatCommandStore(&server->GetApp()),
	m_server(server)
	{
		m_chatEnteringBox = nullptr;
		m_chatLines.resize(maxChatLines);

		m_chatBox = canvas->Add<Ndk::TextAreaWidget>();
		m_chatBox->EnableBackground(false);
		//m_chatBox->SetBackgroundColor(Nz::Color(70, 8, 15, 20));
		m_chatBox->SetSize({ 640.f, maxChatLines * 30.f });
		m_chatBox->SetTextColor(Nz::Color::White);
		m_chatBox->SetReadOnly(true);

		m_chatEnteringBox = canvas->Add<Ndk::TextAreaWidget>();
		m_chatEnteringBox->EnableBackground(true);
		m_chatEnteringBox->SetBackgroundColor(Nz::Color(0, 0, 0, 150));
		m_chatEnteringBox->SetTextColor(Nz::Color::White);
		m_chatEnteringBox->Show(false);

		Nz::EventHandler& eventHandler = window.GetEventHandler();

		// Connect every slot
		m_onChatMessageSlot.Connect(server->OnChatMessage, this, &MatchChatbox::OnChatMessage);
		m_onKeyPressedSlot.Connect(eventHandler.OnKeyPressed, this, &MatchChatbox::OnKeyPressed);
		m_onTargetChangeSizeSlot.Connect(window.OnRenderTargetSizeChange, this, &MatchChatbox::OnRenderTargetSizeChange);

		OnRenderTargetSizeChange(&window);
	}

	MatchChatbox::~MatchChatbox()
	{
		m_chatBox->Destroy();
		if (m_chatEnteringBox)
			m_chatEnteringBox->Destroy();
	}

	void MatchChatbox::PrintMessage(const std::string& message)
	{
		std::cout << message << std::endl;

		m_chatLines.emplace_back(message);
		if (m_chatLines.size() > maxChatLines)
			m_chatLines.erase(m_chatLines.begin());

		m_chatBox->Clear();
		for (const Nz::String& message : m_chatLines)
			m_chatBox->AppendText(message + "\n");
	}

	void MatchChatbox::OnChatMessage(ServerConnection* /*server*/, const Packets::ChatMessage& chatMessage)
	{
		PrintMessage(chatMessage.message);
	}

	void MatchChatbox::OnKeyPressed(const Nz::EventHandler* /*eventHandler*/, const Nz::WindowEvent::KeyEvent& event)
	{
		if (event.code == Nz::Keyboard::Return)
		{
			if (m_chatEnteringBox->IsVisible())
			{
				Nz::String text = m_chatEnteringBox->GetText();
				m_chatEnteringBox->Clear();
				m_chatEnteringBox->Show(false);

				if (!text.IsEmpty())
				{
					std::string chatText = text.ToStdString();
					if (chatText[0] == '/')
					{
						std::string_view command = chatText;
						command.remove_prefix(1);

						std::optional<bool> result = m_chatCommandStore.ExecuteCommand(m_server, command);
						if (result && !result.value())
							PrintMessage(chatText);
						else
						{
							Packets::PlayerChat chat;
							chat.text = std::move(chatText);

							m_server->SendPacket(chat);
						}
					}
					else
					{
						Packets::PlayerChat chat;
						chat.text = std::move(chatText);

						m_server->SendPacket(chat);
					}
				}
			}
			else
			{
				m_chatEnteringBox->Show(true);
				m_chatEnteringBox->SetFocus();
			}
		}
	}

	void MatchChatbox::OnRenderTargetSizeChange(const Nz::RenderTarget* renderTarget)
	{
		Nz::Vector2f size = Nz::Vector2f(renderTarget->GetSize());

		m_chatBox->SetPosition({ 5.f, size.y - 30.f - m_chatBox->GetSize().y, 0.f });
		m_chatEnteringBox->SetSize({ size.x, 40.f });
		m_chatEnteringBox->SetPosition({ 0.f, size.y - m_chatEnteringBox->GetSize().y - 5.f, 0.f });
	}
}
