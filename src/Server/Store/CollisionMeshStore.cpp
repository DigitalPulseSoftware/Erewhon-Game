// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Store/CollisionMeshStore.hpp>
#include <Server/Database/Database.hpp>
#include <Server/Database/DatabaseResult.hpp>
#include <iostream>

namespace ewn
{
	bool CollisionMeshStore::FillStore(ServerApplication* app, DatabaseResult& result)
	{
		assert(result.IsValid());

		std::size_t meshCount = result.GetRowCount();
		Nz::Int32 highestModuleId = std::get<Nz::Int32>(result.GetValue(0, meshCount - 1));

		m_collisionInfos.clear();
		m_collisionInfos.resize(highestModuleId + 1);

		Nz::MeshParams params;
		params.animated = false;
		params.center = true;
		params.optimizeIndexBuffers = false;
		params.storage = Nz::DataStorage_Software;
		//params.vertexDeclaration = Nz::VertexDeclaration::Get(Nz::VertexLayout_XYZ);

		std::size_t meshLoaded = 0;
		for (std::size_t i = 0; i < meshCount; ++i)
		{
			Nz::Int32 id = std::get<Nz::Int32>(result.GetValue(0, i));

			try
			{
				CollisionMeshInfo& collisionInfo = m_collisionInfos[id];
				collisionInfo.doesExist = true;

				std::string meshPath = std::get<std::string>(result.GetValue(1, i));

				collisionInfo.collisionMesh = Nz::Mesh::New();
				if (!collisionInfo.collisionMesh->LoadFromFile("Assets/" + meshPath, params))
					throw std::runtime_error("Failed to load " + meshPath);

				collisionInfo.isLoaded = true;
				meshLoaded++;
			}
			catch (const std::exception& e)
			{
				std::cerr << "Failed to load collision mesh #" << id << ": " << e.what() << std::endl;
			}
		}

		std::cout << "Loaded " << meshLoaded << " collision meshes (" << (meshCount - meshLoaded) << " errored)" << std::endl;

		return true;
	}
}
