// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Store/ModuleStore.hpp>
#include <Server/Database/Database.hpp>
#include <Server/Database/DatabaseResult.hpp>
#include <Server/Modules/CommunicationsModule.hpp>
#include <Server/Modules/EngineModule.hpp>
#include <Server/Modules/NavigationModule.hpp>
#include <Server/Modules/RadarModule.hpp>
#include <Server/Modules/PlasmaBeamWeaponModule.hpp>
#include <Server/Modules/TorpedoWeaponModule.hpp>
#include <iostream>

namespace ewn
{
	std::shared_ptr<SpaceshipModule> ModuleStore::BuildModule(std::size_t moduleId, SpaceshipCore* core, const Ndk::EntityHandle& spaceship) const
	{
		assert(moduleId < m_moduleInfos.size());

		const ModuleInfo& moduleInfo = m_moduleInfos[moduleId];
		if (!moduleInfo.doesExist)
		{
			std::cerr << "Failed to build module: module #" << moduleId << " does not exist" << std::endl;
			return {};
		}

		if (!moduleInfo.isLoaded)
		{
			std::cerr << "Failed to build module: module #" << moduleId << " exists but failed to load" << std::endl;
			return {};
		}

		std::shared_ptr<SpaceshipModule> modulePtr;
		try
		{
			auto it = m_factory.find(moduleInfo.className);
			assert(it != m_factory.end());

			modulePtr = it->second.factoryFunc(core, spaceship, moduleInfo.classInfo);
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to build module: module #" << moduleId << " has invalid info: " << e.what() << std::endl;
			return {};
		}

		return modulePtr;
	}

	void ModuleStore::BuildFactory()
	{
		struct RadarInfo
		{
			float detectionRadius;
			std::size_t maxLockableTarget;
		};

		auto NoInfo = [](const nlohmann::json& /*classInfo*/) { return std::any{}; };

		RegisterModule("communications", NoInfo, [](SpaceshipCore* core, const Ndk::EntityHandle& spaceship, const std::any& /*classInfo*/)
		{
			return std::make_shared<CommunicationsModule>(core, spaceship);
		});

		RegisterModule("engine", NoInfo, [](SpaceshipCore* core, const Ndk::EntityHandle& spaceship, const std::any& /*classInfo*/)
		{
			return std::make_shared<EngineModule>(core, spaceship);
		});

		RegisterModule("navigation", NoInfo, [](SpaceshipCore* core, const Ndk::EntityHandle& spaceship, const std::any& /*classInfo*/)
		{
			return std::make_shared<NavigationModule>(core, spaceship);
		});

		RegisterModule("radar", [](const nlohmann::json& classInfo) -> std::any
		{
			RadarInfo radarInfo;
			radarInfo.detectionRadius = classInfo.at("detectionRadius");
			radarInfo.maxLockableTarget = classInfo.at("maxLockableTarget");

			return radarInfo;
		},
		[](SpaceshipCore* core, const Ndk::EntityHandle& spaceship, const std::any& classInfo)
		{
			const RadarInfo& radarInfo = std::any_cast<const RadarInfo>(classInfo);

			return std::make_shared<RadarModule>(core, spaceship, radarInfo.detectionRadius, radarInfo.maxLockableTarget);
		});

		RegisterModule("weapon_plasmabeam", NoInfo, [](SpaceshipCore* core, const Ndk::EntityHandle& spaceship, const std::any& /*classInfo*/)
		{
			return std::make_shared<PlasmaBeamWeaponModule>(core, spaceship);
		});

		RegisterModule("weapon_torpedo", NoInfo, [](SpaceshipCore* core, const Ndk::EntityHandle& spaceship, const std::any& /*classInfo*/)
		{
			return std::make_shared<TorpedoWeaponModule>(core, spaceship);
		});
	}

	bool ModuleStore::FillStore(ServerApplication* app, DatabaseResult& result)
	{
		assert(result.IsValid());

		std::size_t moduleCount = result.GetRowCount();
		Nz::Int32 highestModuleId = std::get<Nz::Int32>(result.GetValue(0, moduleCount - 1));

		m_moduleInfos.clear();
		m_moduleInfos.resize(highestModuleId + 1);

		std::size_t moduleLoaded = 0;
		for (std::size_t i = 0; i < moduleCount; ++i)
		{
			Nz::Int32 id = std::get<Nz::Int32>(result.GetValue(0, i));

			try
			{
				ModuleInfo& moduleInfo = m_moduleInfos[id];
				moduleInfo.doesExist = true;

				moduleInfo.className = std::get<std::string>(result.GetValue(3, i));
				moduleInfo.name = std::get<std::string>(result.GetValue(1, i));
				moduleInfo.description = std::get<std::string>(result.GetValue(2, i));

				nlohmann::json jsonClassInfo = std::get<nlohmann::json>(result.GetValue(4, i));

				auto it = m_factory.find(moduleInfo.className);
				if (it == m_factory.end())
					throw std::runtime_error("Class name \"" + moduleInfo.className + "\" does not exist");

				moduleInfo.classInfo = it->second.decodeFunc(jsonClassInfo);
				moduleInfo.type = static_cast<ModuleType>(std::get<Nz::Int16>(result.GetValue(5, i)));

				moduleInfo.isLoaded = true;
				moduleLoaded++;
				m_moduleIndices.emplace(moduleInfo.name, id);
			}
			catch (const std::exception& e)
			{
				std::cerr << "Failed to load module #" << id << ": " << e.what() << std::endl;
			}
		}

		std::cout << "Loaded " << moduleLoaded << " modules (" << (moduleCount - moduleLoaded) << " errored)" << std::endl;

		return true;
	}
}
