// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Player.hpp>
#include <Nazara/Network/NetPacket.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <cassert>

namespace ewn
{
	inline void Player::ClearBots()
	{
		m_botEntities.clear();
	}

	inline void Player::Disconnect(Nz::UInt32 data)
	{
		m_session->Disconnect(data);
	}

	inline ServerApplication* Player::GetApp() const
	{
		return m_app;
	}

	inline Arena* Player::GetArena() const
	{
		return m_arena;
	}

	const Ndk::EntityHandle& Player::GetControlledEntity() const
	{
		return m_controlledEntity;
	}

	inline Nz::Int32 Player::GetDatabaseId() const
	{
		return m_databaseId;
	}

	inline const std::string& Player::GetLogin() const
	{
		return m_login;
	}

	inline Nz::UInt16 Player::GetPermissionLevel() const
	{
		return m_permissionLevel;
	}

	inline const std::string& Player::GetName() const
	{
		return m_displayName;
	}

	inline ClientSession* Player::GetSession()
	{
		return m_session;
	}

	inline const ClientSession* Player::GetSession() const
	{
		return m_session;
	}

	inline std::size_t Player::GetSessionId() const
	{
		if (m_session)
			return m_session->GetSessionId();
		else
			return InvalidSessionId;
	}

	inline bool Player::IsAuthenticated() const
	{
		return m_authenticated;
	}

	template<typename T>
	void Player::SendPacket(const T& packet)
	{
		if (!m_session)
			return;

		m_session->SendPacket(packet);
	}
}
