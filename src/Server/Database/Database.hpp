// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_DATABASE_HPP
#define EREWHON_SERVER_DATABASE_HPP

#include <Nazara/Prerequisites.hpp>
#include <Server/Database/DatabaseConnection.hpp>
#include <string>

namespace ewn
{
	class Database
	{
		public:
			inline Database(std::string name, std::string dbHost, Nz::UInt16 port, std::string dbUser, std::string dbPassword, std::string dbName);
			~Database() = default;

			DatabaseConnection CreateConnection();

		private:
			std::string m_name;
			std::string m_dbHostname;
			std::string m_dbPassword;
			std::string m_dbName;
			std::string m_dbUsername;
			Nz::UInt16 m_dbPort;
	};
}

#include <Server/Database/Database.inl>

#endif // EREWHON_SERVER_DATABASE_HPP
