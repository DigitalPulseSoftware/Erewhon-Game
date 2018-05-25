// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_COLLISIONMESHSTORE_HPP
#define EREWHON_SERVER_COLLISIONMESHSTORE_HPP

#include <Server/DatabaseStore.hpp>
#include <Nazara/Physics3D/Collider3D.hpp>
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

			inline const Nz::Collider3DRef& GetEntryCollider(std::size_t entryId) const;
			inline const Nz::Boxf& GetEntryDimensions(std::size_t entryId) const;
			inline std::size_t GetEntryCount() const;
			inline const std::string& GetEntryFilePath(std::size_t entryId) const;
			inline bool IsEntryLoaded(std::size_t entryId) const;

		private:
			bool FillStore(ServerApplication* app, DatabaseResult& result) override;

			struct CollisionMeshInfo 
			{
				Nz::Boxf dimensions;
				Nz::Collider3DRef collider;
				std::string filePath;
				bool doesExist = false;
				bool isLoaded = false;
			};

			std::vector<CollisionMeshInfo> m_collisionInfos;
	};
}

#include <Server/Store/CollisionMeshStore.inl>

#endif // EREWHON_SERVER_COLLISIONMESHSTORE_HPP
