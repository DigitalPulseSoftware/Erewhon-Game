// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Player.hpp>
#include <Nazara/Network/NetPacket.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <cassert>

namespace ewn
{
	inline void Player::Disconnect(Nz::UInt32 data)
	{
		m_networkReactor.DisconnectPeer(m_peerId, data);
	}

	inline Arena* Player::GetArena() const
	{
		return m_arena;
	}

	inline const Ndk::EntityHandle& Player::GetBotEntity() const
	{
		return m_botEntity;
	}

	const Ndk::EntityHandle& Player::GetControlledSpaceship() const
	{
		return m_spaceship;
	}

	inline Nz::UInt32 Player::GetDatabaseId() const
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

	inline std::size_t Player::GetPeerId() const
	{
		return m_peerId;
	}

	inline bool Player::IsAuthenticated() const
	{
		return m_authenticated;
	}

	template<typename T>
	void Player::SendPacket(const T& packet)
	{
		const auto& command = m_commandStore.GetOutgoingCommand<T>();
		
		Nz::NetPacket data;
		m_commandStore.SerializePacket(data, packet);

		m_networkReactor.SendData(m_peerId, command.channelId, command.flags, std::move(data));
	}
}
