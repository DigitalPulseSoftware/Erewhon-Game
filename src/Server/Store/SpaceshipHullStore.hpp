// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_SPACESHIPHULLSTORE_HPP
#define EREWHON_SERVER_SPACESHIPHULLSTORE_HPP

#include <Server/DatabaseStore.hpp>
#include <NDK/Entity.hpp>
#include <string>
#include <vector>

namespace ewn
{
	class Database;
	class DatabaseResult;

	class SpaceshipHullStore final : public DatabaseStore
	{
		public:
			inline SpaceshipHullStore();
			~SpaceshipHullStore() = default;

		private:
			bool FillStore(ServerApplication* app, DatabaseResult& result) override;

			struct HullInfo 
			{
				std::size_t collisionMeshId;
				std::string name;
				std::string description;
				bool doesExist = false;
				bool isLoaded = false;
			};

			std::vector<HullInfo> m_hullInfos;
			bool m_isLoaded;
	};
}

#include <Server/Store/SpaceshipHullStore.inl>

#endif // EREWHON_SERVER_SPACESHIPHULLSTORE_HPP
