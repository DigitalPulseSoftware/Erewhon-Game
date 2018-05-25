// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Store/CollisionMeshStore.hpp>
#include <Nazara/Utility/Mesh.hpp>
#include <Nazara/Utility/StaticMesh.hpp>
#include <Nazara/Utility/VertexDeclaration.hpp>
#include <Nazara/Utility/VertexMapper.hpp>
#include <Server/ServerApplication.hpp>
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

		const std::string& assetsFolder = app->GetConfig().GetStringOption("AssetsFolder");

		std::size_t meshLoaded = 0;
		for (std::size_t i = 0; i < meshCount; ++i)
		{
			Nz::Int32 id = std::get<Nz::Int32>(result.GetValue(0, i));

			try
			{
				CollisionMeshInfo& collisionInfo = m_collisionInfos[id];
				collisionInfo.doesExist = true;

				collisionInfo.filePath = std::get<std::string>(result.GetValue(1, i));

				float scale = std::get<float>(result.GetValue(2, i));
				params.matrix = Nz::Matrix4f::Transform(Nz::Vector3f::Zero(), Nz::EulerAnglesf(0.f, 90.f, 0.f), Nz::Vector3f(scale));

				Nz::Mesh mesh;
				if (!mesh.LoadFromFile(assetsFolder + '/' + collisionInfo.filePath, params))
					throw std::runtime_error("Failed to load " + collisionInfo.filePath);

				std::size_t subMeshCount = mesh.GetSubMeshCount();
				if (subMeshCount > 1)
				{
					// Multiple submeshes, build a compound collider

					/*std::vector<Nz::Collider3DRef> colliders;
					for (std::size_t i = 0; i < subMeshCount; ++i)
					{
						Nz::VertexMapper vertexMapper(mesh.GetSubMesh(i), Nz::BufferAccess_ReadOnly);
						Nz::SparsePtr<Nz::Vector3f> vertices = vertexMapper.GetComponentPtr<Nz::Vector3f>(Nz::VertexComponent_Position);

						colliders.emplace_back(Nz::ConvexCollider3D::New(vertices, vertexMapper.GetVertexCount(), 0.01f));
					}

					collisionInfo.collider = Nz::CompoundCollider3D::New(std::move(colliders));*/

					// Build a convex collider out of every mesh vertices
					std::vector<Nz::Vector3f> vertices;
					for (std::size_t i = 0; i < subMeshCount; ++i)
					{
						Nz::VertexMapper vertexMapper(mesh.GetSubMesh(i), Nz::BufferAccess_ReadOnly);
						Nz::SparsePtr<Nz::Vector3f> subMeshVertices = vertexMapper.GetComponentPtr<Nz::Vector3f>(Nz::VertexComponent_Position);

						Nz::UInt32 vertexCount = vertexMapper.GetVertexCount();
						vertices.reserve(vertices.size() + vertexCount);
						for (Nz::UInt32 i = 0; i < vertexCount; ++i)
							vertices.push_back(subMeshVertices[i]);
					}

					collisionInfo.collider = Nz::ConvexCollider3D::New(vertices.data(), vertices.size(), 0.01f);
				}
				else
				{
					// One submesh, build one convex collider
					Nz::VertexMapper vertexMapper(mesh.GetSubMesh(0), Nz::BufferAccess_ReadOnly);
					Nz::SparsePtr<Nz::Vector3f> vertices = vertexMapper.GetComponentPtr<Nz::Vector3f>(Nz::VertexComponent_Position);

					collisionInfo.collider = Nz::ConvexCollider3D::New(vertices, vertexMapper.GetVertexCount(), 0.01f);
				}

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
