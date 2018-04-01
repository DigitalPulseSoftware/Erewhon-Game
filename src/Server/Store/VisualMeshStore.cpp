// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Store/VisualMeshStore.hpp>
#include <Server/Database/Database.hpp>
#include <Server/Database/DatabaseResult.hpp>
#include <iostream>

namespace ewn
{
	bool VisualMeshStore::FillStore(ServerApplication* app, DatabaseResult& result)
	{
		assert(result.IsValid());

		std::size_t meshCount = result.GetRowCount();
		Nz::Int32 highestModuleId = std::get<Nz::Int32>(result.GetValue(0, meshCount - 1));

		m_visualInfos.clear();
		m_visualInfos.resize(highestModuleId + 1);

		std::size_t meshLoaded = 0;
		for (std::size_t i = 0; i < meshCount; ++i)
		{
			Nz::Int32 id = std::get<Nz::Int32>(result.GetValue(0, i));

			try
			{
				VisualMeshInfo& visualInfo = m_visualInfos[id];
				visualInfo.doesExist = true;

				visualInfo.filePath = std::get<std::string>(result.GetValue(1, i));

				visualInfo.isLoaded = true;
				meshLoaded++;
			}
			catch (const std::exception& e)
			{
				std::cerr << "Failed to load visual mesh #" << id << ": " << e.what() << std::endl;
			}
		}

		std::cout << "Loaded " << meshLoaded << " visual meshes (" << (meshCount - meshLoaded) << " errored)" << std::endl;

		return true;
	}
}
