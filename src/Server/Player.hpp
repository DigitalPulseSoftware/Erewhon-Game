// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_PLAYER_HPP
#define EREWHON_SERVER_PLAYER_HPP

#include <Nazara/Core/HandledObject.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Math/Box.hpp>
#include <NDK/EntityOwner.hpp>
#include <Shared/NetworkReactor.hpp>
#include <Server/ClientSession.hpp>
#include <Server/ServerCommandStore.hpp>

namespace ewn
{
	class Arena;

	class Player;

	using PlayerHandle = Nz::ObjectHandle<Player>;

	class Player : public Nz::HandledObject<Player>
	{
		public:
			struct FleetData;

			Player(ServerApplication* app);
			~Player();

			void Authenticate(Nz::Int32 dbId, std::function<void (Player*, bool succeeded)> authenticationCallback);

			bool CanShoot() const;

			inline void ClearBots();
			void ClearControlledEntity();

			void CreateSpaceship(std::string name, std::string code, std::size_t hullId, std::vector<std::size_t> modules, std::function<void(Player*, bool succeded)> creationCallback);

			inline void Disconnect(Nz::UInt32 data = 0);

			inline ServerApplication* GetApp() const;
			inline Arena* GetArena() const;
			inline const Ndk::EntityHandle& GetControlledEntity() const;
			inline Nz::Int32 GetDatabaseId() const;
			void GetFleetData(const std::string& fleetName, std::function<void(bool found, const FleetData& fleet)> callback);
			Nz::UInt64 GetLastInputProcessedTime() const;
			inline const std::string& GetLogin() const;
			inline Nz::UInt16 GetPermissionLevel() const;
			inline const std::string& GetName() const;
			inline ClientSession* GetSession();
			inline const ClientSession* GetSession() const;
			inline std::size_t GetSessionId() const;

			const Ndk::EntityHandle& InstantiateBot(const std::string& name, std::size_t spaceshipHullId, Nz::Vector3f positionOffset = Nz::Vector3f::Zero());

			inline bool IsAuthenticated() const;

			void MoveToArena(Arena* arena);

			void PrintMessage(std::string chatMessage);

			template<typename T> void SendPacket(const T& packet);

			void Shoot();

			void Update(float elapsedTime);

			void UpdateControlledEntity(const Ndk::EntityHandle& entity);
			void UpdateInput(Nz::UInt64 time, Nz::Vector3f direction, Nz::Vector3f rotation);
			void UpdatePermissionLevel(Nz::UInt16 permissionLevel, std::function<void(bool updateSucceeded)> databaseCallback = nullptr);
			void UpdateSession(ClientSession* session);

			struct FleetData
			{
				struct SpaceshipType
				{
					Nz::Boxf dimensions;
					Nz::Int32 spaceshipId;
					std::size_t hullId;
					std::size_t collisionMeshId;
					std::string script;
					std::string name;
					std::vector<std::size_t> modules;
				};

				struct Spaceship
				{
					Nz::Vector3f position;
					std::size_t spaceshipType;
				};

				std::size_t fleetId;
				std::string fleetName;
				std::vector<Spaceship> spaceships;
				std::vector<SpaceshipType> spaceshipTypes;
			};

			static constexpr std::size_t InvalidSessionId = std::numeric_limits<std::size_t>::max();

		private:
			void OnAuthenticated(std::string login, std::string displayName, Nz::UInt16 permissionLevel);

			struct NoAction
			{
			};

			struct ShootAction
			{
			};

			Arena* m_arena;
			ClientSession* m_session;
			ServerApplication* m_app;
			std::string m_displayName;
			std::string m_login;
			std::variant<NoAction, ShootAction> m_pendingAction;
			std::vector<Ndk::EntityOwner> m_botEntities;
			Ndk::EntityHandle m_controlledEntity;
			Nz::Int32 m_databaseId;
			Nz::UInt16 m_permissionLevel;
			Nz::UInt64 m_lastInputTime;
			Nz::UInt64 m_lastShootTime;
			bool m_authenticated;
	};
}

#include <Server/Player.inl>

#endif // EREWHON_SERVER_PLAYER_HPP
