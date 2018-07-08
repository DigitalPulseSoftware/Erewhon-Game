// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerChatCommandStore.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <Server/Arena.hpp>
#include <Server/Player.hpp>
#include <Server/ServerApplication.hpp>
#include <Server/Components/HealthComponent.hpp>
#include <Server/Components/ScriptComponent.hpp>

namespace ewn
{
	bool ChatCommandProcessArg(Player* player, std::string_view& cmdArgs, Player** arg, Nz::TypeTag<Player*>)
	{
		std::string playerName;
		if (!ChatCommandProcessArg(player, cmdArgs, &playerName, Nz::TypeTag<std::string>()))
			return false;

		if (Player* targetPlayer = player->GetArena()->FindPlayerByName(playerName))
		{
			*arg = targetPlayer;
			return true;
		}
		else
			return false;
	}

	void ServerChatCommandStore::BuildStore(ServerApplication* /*app*/)
	{
		RegisterCommand("clearbots", &ServerChatCommandStore::HandleClearBots);
		RegisterCommand("crashserver", &ServerChatCommandStore::HandleCrashServer);
		RegisterCommand("debugparticles", &ServerChatCommandStore::HandleDebugParticles);
		RegisterCommand("kamikaze", &ServerChatCommandStore::HandleSuicide);
		RegisterCommand("kick", &ServerChatCommandStore::HandleKickPlayer);
		RegisterCommand("reloadarena", &ServerChatCommandStore::HandleReloadArena);
		RegisterCommand("reloadmodules", &ServerChatCommandStore::HandleReloadModules);
		RegisterCommand("resetarena", &ServerChatCommandStore::HandleResetArena);
		RegisterCommand("spawnfleet", &ServerChatCommandStore::HandleSpawnFleet);
		RegisterCommand("stopserver", &ServerChatCommandStore::HandleStopServer);
		RegisterCommand("suicide", &ServerChatCommandStore::HandleSuicide);
		RegisterCommand("spawnbot", &ServerChatCommandStore::HandleSpawnBot);
		RegisterCommand("updatepermission", &ServerChatCommandStore::HandleUpdatePermission);
	}

	bool ServerChatCommandStore::HandleClearBots(ServerApplication* /*app*/, Player* player)
	{
		player->ClearBots();

		return true;
	}

	bool ServerChatCommandStore::HandleCrashServer(ServerApplication* /*app*/, Player* player)
	{
		if (player->GetPermissionLevel() < 40)
			return false;

		*static_cast<volatile int*>(nullptr) = 42;

		return true;
	}

	bool ServerChatCommandStore::HandleDebugParticles(ServerApplication* app, Player* player, unsigned int particleSystemId)
	{
		if (const Ndk::EntityHandle& playerSpaceship = player->GetControlledEntity())
		{
			auto& spaceshipNode = playerSpaceship->GetComponent<Ndk::NodeComponent>();

			Nz::Vector3f targetPos = spaceshipNode.GetPosition() + spaceshipNode.GetForward() * 10.f;

			Packets::InstantiateParticleSystem packet;
			packet.particleSystemId = particleSystemId;
			packet.position = targetPos;
			packet.rotation = spaceshipNode.GetRotation();
			packet.scale = Nz::Vector3f(1.f);

			player->GetArena()->BroadcastPacket(packet);
		}

		return false;
	}

	bool ServerChatCommandStore::HandleKickPlayer(ServerApplication* app, Player* player, Player* target)
	{
		if (player->GetPermissionLevel() < 30)
			return false;

		if (target->GetPermissionLevel() >= player->GetPermissionLevel())
		{
			player->PrintMessage("You're not allowed to do that");
			return false;
		}

		target->Disconnect();
		return true;
	}

	bool ServerChatCommandStore::HandleReloadArena(ServerApplication * app, Player * player)
	{
		if (player->GetPermissionLevel() < 30)
			return false;

		if (Arena* arena = player->GetArena())
			arena->Reload();

		return true;
	}

	bool ServerChatCommandStore::HandleReloadModules(ServerApplication* app, Player* player)
	{
		if (player->GetPermissionLevel() < 30)
			return false;

		ModuleStore& moduleStore = app->GetModuleStore();
		moduleStore.LoadFromDatabase(app, app->GetGlobalDatabase(), [ply = player->CreateHandle()](bool updateSucceeded)
		{
			if (updateSucceeded)
				ply->PrintMessage("Module reloaded");
			else
				ply->PrintMessage("Failed to reload modules");
		});

		return true;
	}

	bool ServerChatCommandStore::HandleResetArena(ServerApplication* /*app*/, Player* player)
	{
		if (player->GetPermissionLevel() < 20)
			return false;

		if (Arena* arena = player->GetArena())
			arena->Reset();

		return true;
	}

	bool ServerChatCommandStore::HandleSpawnFleet(ServerApplication* app, Player* player, std::string fleetName)
	{
		if (player->GetPermissionLevel() < 40)
			return false;

		if (Arena* arena = player->GetArena())
			arena->SpawnFleet(player, fleetName);

		return true;
	}

	bool ServerChatCommandStore::HandleSpawnBot(ServerApplication* app, Player* player, std::string spaceshipName, std::size_t spaceshipCount)
	{
		if (player->GetPermissionLevel() < 40)
			return false;

		if (spaceshipCount < 1 || spaceshipCount > 10)
		{
			player->PrintMessage("Invalid count, must be in range [1,10]");
			return false;
		}

		app->GetGlobalDatabase().ExecuteStatement("FindSpaceshipByOwnerIdAndName", { Nz::Int32(player->GetDatabaseId()), spaceshipName }, [app, spaceshipCount, sessionId = player->GetSessionId(), spaceshipName](DatabaseResult& result)
		{
			if (!result)
				std::cerr << "Find spaceship query failed: " << result.GetLastErrorMessage() << std::endl;

			Player* ply = app->GetPlayerBySession(sessionId);
			if (!ply)
				return;

			if (!result)
			{
				ply->PrintMessage("Failed to spawn spaceship \"" + spaceshipName + "\", please contact an admin");
				return;
			}

			if (result.GetRowCount() == 0)
			{
				ply->PrintMessage("You have no spaceship named \"" + spaceshipName + "\"");
				return;
			}

			Nz::Int32 spaceshipId = std::get<Nz::Int32>(result.GetValue(0));
			std::string code = std::get<std::string>(result.GetValue(1));
			Nz::Int32 spaceshipHullId = std::get<Nz::Int32>(result.GetValue(2));

			app->GetGlobalDatabase().ExecuteStatement("FindSpaceshipModulesBySpaceshipId", { spaceshipId }, [app, ply, spaceshipHullId, spaceshipCount, shipName = std::move(spaceshipName), spaceshipCode = std::move(code)](DatabaseResult& result)
			{
				if (!result)
					std::cerr << "Find spaceship modules failed: " << result.GetLastErrorMessage() << std::endl;

				if (!ply)
					return;

				if (!result)
				{
					ply->PrintMessage("Failed to retrieve spaceship modules, please contact an administrator");
					return;
				}

				std::size_t moduleCount = result.GetRowCount();

				std::vector<std::size_t> moduleIds(moduleCount);
				try
				{
					for (std::size_t i = 0; i < moduleCount; ++i)
						moduleIds[i] = static_cast<std::size_t>(std::get<Nz::Int32>(result.GetValue(0, i)));
				}
				catch (const std::exception& e)
				{
					std::cerr << "Failed to retrieve spaceship modules: " << e.what() << std::endl;

					ply->PrintMessage("Failed to retrieve spaceship modules, please contact an administrator");
					return;
				}

				for (std::size_t i = 0; i < spaceshipCount; ++i)
				{
					const Ndk::EntityHandle& playerBot = ply->InstantiateBot(shipName, spaceshipHullId, float(i) * Nz::Vector3f::Right() * 10.f);
					ScriptComponent& botScript = playerBot->AddComponent<ScriptComponent>();
					if (!botScript.Initialize(app, moduleIds))
					{
						ply->PrintMessage("Failed to initialize bot #" + std::to_string(i) + ", please contact an administrator");
						return;
					}

					Nz::String lastError;
					if (!botScript.Execute(spaceshipCode, &lastError))
						ply->PrintMessage("Failed to execute script for bot #" + std::to_string(i) + ": " + lastError.ToStdString());
				}

				ply->PrintMessage("Bot(s) loaded with success");
			});
		});

		return true;
	}

	bool ServerChatCommandStore::HandleSuicide(ServerApplication* /*app*/, Player* player)
	{
		if (const Ndk::EntityHandle& playerSpaceship = player->GetControlledEntity())
		{
			HealthComponent& spaceshipHealth = playerSpaceship->GetComponent<HealthComponent>();
			spaceshipHealth.Damage(spaceshipHealth.GetHealth(), playerSpaceship);
		}

		return true;
	}

	bool ServerChatCommandStore::HandleStopServer(ServerApplication* app, Player* player)
	{
		if (player->GetPermissionLevel() < 40)
			return false;

		app->Quit();
		return true;
	}

	bool ServerChatCommandStore::HandleUpdatePermission(ServerApplication* app, Player* player, Player* target, Nz::UInt16 permissionLevel)
	{
		if (permissionLevel >= player->GetPermissionLevel())
		{
			player->PrintMessage("You're not allowed to do that");
			return false;
		}

		if (target->GetPermissionLevel() >= player->GetPermissionLevel())
		{
			player->PrintMessage("You're not allowed to do that");
			return false;
		}

		target->UpdatePermissionLevel(permissionLevel, [ply = player->CreateHandle()](bool success)
		{
			if (!ply)
				return;

			if (success)
				ply->PrintMessage("Permission level was successfully updated");
			else
				ply->PrintMessage("Failed to update permission level in database, changes are local");
		});

		return false;
	}
}
