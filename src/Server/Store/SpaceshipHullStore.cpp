// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Store/SpaceshipHullStore.hpp>
#include <Server/ServerApplication.hpp>
#include <Server/Database/Database.hpp>
#include <Server/Database/DatabaseResult.hpp>
#include <Server/Store/CollisionMeshStore.hpp>
#include <iostream>

namespace ewn
{
	bool SpaceshipHullStore::FillStore(ServerApplication* app, DatabaseResult& result)
	{
		assert(result.IsValid());

		std::size_t hullCount = result.GetRowCount();
		Nz::Int32 highestModuleId = std::get<Nz::Int32>(result.GetValue(0, hullCount - 1));

		m_hullInfos.clear();
		m_hullInfos.resize(highestModuleId + 1);

		CollisionMeshStore& collisionMeshStore = app->GetCollisionMeshStore();
		assert(collisionMeshStore.IsLoaded());

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
				hullInfo.collisionMeshId = static_cast<std::size_t>(std::get<Nz::Int32>(result.GetValue(3, i)));
				hullInfo.visualMeshId = static_cast<std::size_t>(std::get<Nz::Int32>(result.GetValue(4, i)));

				if (!collisionMeshStore.IsEntryLoaded(hullInfo.collisionMeshId))
					throw std::runtime_error("Hull depends on collision mesh #" + std::to_string(hullInfo.collisionMeshId) + " which is not loaded");

				// Load slots
				app->GetGlobalDatabase().ExecuteStatement("LoadSpaceshipHullSlots", { id }, [this, id](DatabaseResult& slotResult) { LoadSlots(id, slotResult); });

				hullInfo.isLoaded = true;
				hullLoaded++;
				m_hullIndices.emplace(hullInfo.name, id);
			}
			catch (const std::exception& e)
			{
				std::cerr << "Failed to load spaceship hull #" << id << ": " << e.what() << std::endl;
			}
		}

		std::cout << "Loaded " << hullLoaded << " spaceship hulls (" << (hullCount - hullLoaded) << " errored)" << std::endl;

		return true;
	}

	void SpaceshipHullStore::LoadSlots(std::size_t hullId, DatabaseResult& result)
	{
		if (!result)
		{
			std::cerr << "LoadSpaceshipHullSlots for hull id " << hullId << " failed: " << result.GetLastErrorMessage();
			return;
		}

		HullInfo& hullInfo = m_hullInfos[hullId];

		std::size_t slotCount = result.GetRowCount();
		hullInfo.slots.reserve(slotCount);

		for (std::size_t i = 0; i < slotCount; ++i)
		{
			SlotInfo& slotInfo = hullInfo.slots.emplace_back();
			slotInfo.moduleType = static_cast<ModuleType>(std::get<Nz::Int16>(result.GetValue(0, i)));
		}
	}
}
