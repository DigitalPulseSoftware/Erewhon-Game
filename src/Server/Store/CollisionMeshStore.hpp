// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_COLLISIONMESHSTORE_HPP
#define EREWHON_SERVER_COLLISIONMESHSTORE_HPP

#include <Server/DatabaseStore.hpp>
#include <Nazara/Utility/Mesh.hpp>
#include <NDK/Entity.hpp>
#include <string>
#include <vector>

namespace ewn
{
	class Database;
	class DatabaseResult;

	class CollisionMeshStore final : public DatabaseStore
	{
		public:
			inline CollisionMeshStore();
			~CollisionMeshStore() = default;

			inline bool IsEntryLoaded(std::size_t entryId) const;

		private:
			bool FillStore(ServerApplication* app, DatabaseResult& result) override;

			struct CollisionMeshInfo 
			{
				Nz::MeshRef collisionMesh;
				bool doesExist = false;
				bool isLoaded = false;
			};

			std::vector<CollisionMeshInfo> m_collisionInfos;
	};
}

#include <Server/Store/CollisionMeshStore.inl>

#endif // EREWHON_SERVER_COLLISIONMESHSTORE_HPP
