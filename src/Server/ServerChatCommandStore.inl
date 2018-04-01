// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerChatCommandStore.hpp>

namespace ewn
{
	inline ServerChatCommandStore::ServerChatCommandStore(ServerApplication* app) :
	ChatCommandStore(app)
	{
		BuildStore(app);
	}
}
