// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_GLOBALDATABASE_HPP
#define EREWHON_SERVER_GLOBALDATABASE_HPP

#include <Server/Database/Database.hpp>

namespace ewn
{
	class GlobalDatabase final : public Database
	{
		friend DatabaseWorker;

		public:
			inline GlobalDatabase(std::string dbHost, Nz::UInt16 port, std::string dbUser, std::string dbPassword, std::string dbName);
			~GlobalDatabase() = default;

		private:
			void PrepareStatements(DatabaseConnection& conn) override;
	};
}

#include <Server/GlobalDatabase.inl>

#endif // EREWHON_SERVER_GLOBALDATABASE_HPP
