// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientApplication.hpp>

namespace ewn
{
	inline Nz::UInt64 ClientApplication::GetServerTimeCetteMethodeEstAussiDegueu() const
	{
		return GetAppTime() + m_deltaTimeDegueux;
	}

	inline void ClientApplication::SetDeltaTimeFromServerToClientCetteMethodeEstDegueuDeTouteFacon(Nz::UInt64 deltaTime)
	{
		m_deltaTimeDegueux = deltaTime;
	}
}
