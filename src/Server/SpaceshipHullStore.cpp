// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/SpaceshipHullStore.hpp>
#include <Server/Database/Database.hpp>
#include <Server/Database/DatabaseResult.hpp>
#include <iostream>

namespace ewn
{
	void SpaceshipHullStore::LoadFromDatabase(Database& database, std::function<void(bool succeeded)> callback)
	{
		database.ExecuteQuery("LoadSpaceshipHulls", {}, [this, cb = std::move(callback)](DatabaseResult& result)
		{
			bool succeeded = HandleDatabaseResult(result);
			if (cb)
				cb(succeeded);
		});
	}

	bool SpaceshipHullStore::HandleDatabaseResult(DatabaseResult& result)
	{
		if (!result.IsValid())
		{
			std::cerr << "Load spaceship hull query failed: " << result.GetLastErrorMessage() << std::endl;
			return false;
		}

		std::size_t hullCount = result.GetRowCount();
		Nz::Int32 highestModuleId = std::get<Nz::Int32>(result.GetValue(0, hullCount - 1));

		m_hullInfos.clear();
		m_hullInfos.resize(highestModuleId + 1);

		Nz::MeshParams params;
		params.animated = false;
		params.center = true;
		params.optimizeIndexBuffers = false;
		params.storage = Nz::DataStorage_Software;

		std::size_t hullLoaded = 0;
		for (std::size_t i = 0; i < hullCount; ++i)
		{
			Nz::Int32 id = std::get<Nz::Int32>(result.GetValue(0, i));

			try
			{
				HullInfo& hullInfo = m_hullInfos[id];
				hullInfo.doesExist = true;

				hullInfo.name = std::get<std::string>(result.GetValue(1, i));
				hullInfo.description = std::get<std::string>(result.GetValue(2, i));

				std::string meshPath = std::get<std::string>(result.GetValue(3, i));

				hullInfo.collisionMesh = Nz::Mesh::New();
				if (!hullInfo.collisionMesh->LoadFromFile("Assets/" + meshPath, params))
					throw std::runtime_error("Failed to load " + meshPath);

				hullInfo.isLoaded = true;
				hullLoaded++;
			}
			catch (const std::exception& e)
			{
				std::cerr << "Failed to load spaceship hull #" << id << ": " << e.what() << std::endl;
			}
		}

		std::cout << "Loaded " << hullLoaded << " spaceship hulls (" << (hullCount - hullLoaded) << " errored)" << std::endl;

		m_isLoaded = true;
		return true;
	}
}
