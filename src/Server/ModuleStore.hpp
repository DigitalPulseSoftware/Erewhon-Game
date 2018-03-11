// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_MODULESTORE_HPP
#define EREWHON_SERVER_MODULESTORE_HPP

#include <NDK/Entity.hpp>
#include <json/json.hpp>
#include <any>
#include <string>
#include <unordered_map>
#include <vector>

namespace ewn
{
	class Database;
	class DatabaseResult;
	class SpaceshipCore;
	class SpaceshipModule;

	class ModuleStore
	{
		public:
			inline ModuleStore();
			~ModuleStore() = default;

			std::shared_ptr<SpaceshipModule> BuildModule(std::size_t moduleId, SpaceshipCore* core, const Ndk::EntityHandle& spaceship) const;

			inline bool IsLoaded() const;

			void LoadFromDatabase(Database& database, std::function<void(bool succeeded)> callback = nullptr);

		private:
			using DecodeClassInfoFunction = std::function<std::any(const nlohmann::json& classInfo)>;
			using FactoryFunction = std::function<std::shared_ptr<SpaceshipModule>(SpaceshipCore* core, const Ndk::EntityHandle& spaceship, const std::any& classInfo)>;

			void BuildFactory();
			bool HandleLoadModules(DatabaseResult& result);

			inline void RegisterModule(std::string className, DecodeClassInfoFunction decodeFunc, FactoryFunction factoryFunc);

			struct FactoryData
			{
				DecodeClassInfoFunction decodeFunc;
				FactoryFunction factoryFunc;
			};

			struct ModuleInfo 
			{
				std::any classInfo;
				std::string className;
				std::string name;
				std::string description;
				bool doesExist = false;
				bool isLoaded = false;
			};

			std::unordered_map<std::string, FactoryData> m_factory;
			std::vector<ModuleInfo> m_moduleInfos;
			bool m_isLoaded;
	};
}

#include <Server/ModuleStore.inl>

#endif // EREWHON_SERVER_MODULESTORE_HPP
