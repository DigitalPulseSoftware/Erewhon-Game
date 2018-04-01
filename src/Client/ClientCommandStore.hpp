// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_COMMANDSTORE_HPP
#define EREWHON_CLIENT_COMMANDSTORE_HPP

#include <Shared/CommandStore.hpp>

namespace ewn
{
	class ServerConnection;

	class ClientCommandStore final : public CommandStore
	{
		public:
			ClientCommandStore(ServerConnection* app);
			~ClientCommandStore() = default;
	};
}

#include <Client/ClientCommandStore.inl>

#endif // EREWHON_CLIENT_COMMANDSTORE_HPP
