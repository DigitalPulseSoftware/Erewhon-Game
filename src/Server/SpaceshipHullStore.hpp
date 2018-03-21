// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_SPACESHIPHULLSTORE_HPP
#define EREWHON_SERVER_SPACESHIPHULLSTORE_HPP

#include <Nazara/Utility/Mesh.hpp>
#include <NDK/Entity.hpp>
#include <json/json.hpp>
#include <string>
#include <vector>

namespace ewn
{
	class Database;
	class DatabaseResult;

	class SpaceshipHullStore
	{
		public:
			inline SpaceshipHullStore();
			~SpaceshipHullStore() = default;

			inline bool IsLoaded() const;

			void LoadFromDatabase(Database& database, std::function<void(bool succeeded)> callback = nullptr);

		private:
			bool HandleDatabaseResult(DatabaseResult& result);

			struct HullInfo 
			{
				Nz::MeshRef collisionMesh;
				std::string name;
				std::string description;
				bool doesExist = false;
				bool isLoaded = false;
			};

			std::vector<HullInfo> m_hullInfos;
			bool m_isLoaded;
	};
}

#include <Server/SpaceshipHullStore.inl>

#endif // EREWHON_SERVER_SPACESHIPHULLSTORE_HPP
