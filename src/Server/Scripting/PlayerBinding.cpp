// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Scripting/ArenaInterface.hpp>
#include <NDK/LuaAPI.hpp>
#include <Server/Arena.hpp>
#include <Server/Player.hpp>
#include <Server/ServerApplication.hpp>

namespace ewn
{
	void ArenaInterface::BindPlayer()
	{
		s_playerBinding.Reset("Player");
		
		s_playerBinding.BindMethod("ClearBots", &Player::ClearBots);
		s_playerBinding.BindMethod("ClearControlledEntity", &Player::ClearControlledEntity);
		s_playerBinding.BindMethod("Disconnect", &Player::Disconnect, Nz::UInt32(0));
		s_playerBinding.BindMethod("GetArena", &Player::GetArena);
		s_playerBinding.BindMethod("GetControlledEntity", &Player::GetControlledEntity);
		s_playerBinding.BindMethod("GetDatabaseId", &Player::GetDatabaseId);
		s_playerBinding.BindMethod("GetLastInputProcessedTime", &Player::GetLastInputProcessedTime);
		s_playerBinding.BindMethod("GetLogin", &Player::GetLogin);
		s_playerBinding.BindMethod("GetName", &Player::GetName);
		s_playerBinding.BindMethod("GetPermissionLevel", &Player::GetPermissionLevel);
		s_playerBinding.BindMethod("GetSessionId", &Player::GetSessionId);
		s_playerBinding.BindMethod("InstantiateBot", &Player::InstantiateBot);
		s_playerBinding.BindMethod("MoveToArena", &Player::MoveToArena);
		s_playerBinding.BindMethod("PrintMessage", &Player::PrintMessage);
		s_playerBinding.BindMethod("Shoot", &Player::Shoot);
		s_playerBinding.BindMethod("UpdateControlledEntity", &Player::UpdateControlledEntity);

		s_playerBinding.BindMethod("GetFleetData", [](Nz::LuaState& state, Player* instance, std::size_t argumentCount) -> int
		{
			int argIndex = 2;
			std::string fleetName = state.Check<std::string>(&argIndex);
			state.CheckType(argIndex, Nz::LuaType_Function);

			int callbackRef = state.CreateReference();

			Arena* arena = instance->GetArena();

			instance->GetFleetData(fleetName, [arena, callbackRef](bool success, const Player::FleetData& fleetData)
			{
				Nz::LuaInstance& luaInstance = arena->GetLuaInstance();
				luaInstance.PushReference(callbackRef);
				if (!luaInstance.IsValid(-1))
				{
					luaInstance.Pop();
					return;
				}

				if (!success)
				{
					luaInstance.Call(0);
					return;
				}

				luaInstance.PushTable(0, 2);
				{
					luaInstance.PushField("fleetId", fleetData.fleetId);

					luaInstance.PushTable(fleetData.spaceships.size(), 0);
					{
						std::size_t spaceshipIndex = 1;
						for (const auto& spaceshipData : fleetData.spaceships)
						{
							luaInstance.PushInteger(spaceshipIndex++);

							luaInstance.PushTable(0, 8);
							{
								luaInstance.PushField("spaceshipId", spaceshipData.spaceshipId);
								luaInstance.PushField("count", spaceshipData.count);
								luaInstance.PushField("hullId", spaceshipData.hullId);
								luaInstance.PushField("collisionMeshId", spaceshipData.collisionMeshId);
								luaInstance.PushField("script", spaceshipData.script);
								luaInstance.PushField("name", spaceshipData.name);

								luaInstance.PushTable(spaceshipData.modules.size(), 0);
								{
									std::size_t moduleIndex = 1;
									for (std::size_t moduleId : spaceshipData.modules)
									{
										luaInstance.Push(moduleIndex++);
										luaInstance.Push(moduleId);
										luaInstance.SetTable();
									}
								}
								luaInstance.SetField("modules");
							}

							luaInstance.SetTable();
						}
					}
					luaInstance.SetField("spaceships");
				}

				luaInstance.Call(1);
			});

			return 0;
		});
	}
}
