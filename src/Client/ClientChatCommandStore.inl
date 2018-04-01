// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientChatCommandStore.hpp>

namespace ewn
{
	inline ClientChatCommandStore::ClientChatCommandStore(ClientApplication* app) :
	ChatCommandStore(app)
	{
		BuildStore(app);
	}
}
