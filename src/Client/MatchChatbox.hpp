// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_MATCHCHATBOX_HPP
#define EREWHON_CLIENT_MATCHCHATBOX_HPP

#include <Nazara/Renderer/RenderWindow.hpp>
#include <NDK/Canvas.hpp>
#include <NDK/Entity.hpp>
#include <NDK/EntityOwner.hpp>
#include <NDK/Widgets/TextAreaWidget.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Client/ClientChatCommandStore.hpp>
#include <Client/ServerConnection.hpp>

namespace ewn
{
	class MatchChatbox
	{
		public:
			MatchChatbox(ServerConnection* server, Nz::RenderWindow& window, Ndk::Canvas* canvas);
			MatchChatbox(const MatchChatbox&) = delete;
			MatchChatbox(MatchChatbox&&) = delete;
			~MatchChatbox();

			inline bool IsTyping() const;

			void PrintMessage(const std::string& message);

			MatchChatbox& operator=(const MatchChatbox&) = delete;
			MatchChatbox& operator=(MatchChatbox&&) = delete;

		private:
			void OnChatMessage(ServerConnection* server, const Packets::ChatMessage& chatMessage);
			void OnKeyPressed(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::KeyEvent& event);
			void OnRenderTargetSizeChange(const Nz::RenderTarget* renderTarget);

			NazaraSlot(ServerConnection, OnChatMessage, m_onChatMessageSlot);
			NazaraSlot(Nz::EventHandler, OnKeyPressed, m_onKeyPressedSlot);
			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			std::vector<Nz::String> m_chatLines;
			Ndk::TextAreaWidget* m_chatBox;
			Ndk::TextAreaWidget* m_chatEnteringBox;
			ServerConnection* m_server;
			ClientChatCommandStore m_chatCommandStore;
	};
}

#include <Client/MatchChatbox.inl>

#endif // EREWHON_CLIENT_MATCHCHATBOX_HPP
