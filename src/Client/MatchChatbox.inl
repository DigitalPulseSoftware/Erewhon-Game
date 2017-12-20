// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/MatchChatbox.hpp>

namespace ewn
{
	inline bool MatchChatbox::IsTyping() const
	{
		return m_chatEnteringBox->IsVisible();
	}
}
