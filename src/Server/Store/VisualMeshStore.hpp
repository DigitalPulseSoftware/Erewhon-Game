// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_VISUALMESHSTORE_HPP
#define EREWHON_SERVER_VISUALMESHSTORE_HPP

#include <Server/DatabaseStore.hpp>
#include <NDK/Entity.hpp>
#include <string>
#include <vector>

namespace ewn
{
	class Database;
	class DatabaseResult;

	class VisualMeshStore final : public DatabaseStore
	{
		public:
			inline VisualMeshStore();
			~VisualMeshStore() = default;

			inline const std::string& GetEntryFilePath(std::size_t entryId) const;
			inline std::size_t GetEntryCount() const;
			inline bool IsEntryLoaded(std::size_t entryId) const;

		private:
			bool FillStore(ServerApplication* app, DatabaseResult& result) override;

			struct VisualMeshInfo 
			{
				std::string filePath;
				bool doesExist = false;
				bool isLoaded = false;
			};

			std::vector<VisualMeshInfo> m_visualInfos;
	};
}

#include <Server/Store/VisualMeshStore.inl>

#endif // EREWHON_SERVER_VISUALMESHSTORE_HPP
