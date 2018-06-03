// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Scripting/ArenaInterface.hpp>

namespace ewn
{
}

namespace Nz
{
	inline unsigned int LuaImplQueryArg(const LuaState& instance, int index, ewn::Arena** arg, TypeTag<ewn::Arena*>)
	{
		if (instance.IsOfType(index, Nz::LuaType_Nil))
			*arg = nullptr;
		else
			*arg = *static_cast<ewn::Arena**>(instance.CheckUserdata(index, "Arena"));

		return 1;
	}

	inline unsigned int LuaImplQueryArg(const LuaState& instance, int index, ewn::Player** arg, TypeTag<ewn::Player*>)
	{
		if (instance.IsOfType(index, Nz::LuaType_Nil))
			*arg = nullptr;
		else
			*arg = static_cast<ewn::PlayerHandle*>(instance.CheckUserdata(index, "Player"))->GetObject();

		return 1;
	}

	inline unsigned int LuaImplQueryArg(const LuaState& instance, int index, ewn::PlayerHandle* arg, TypeTag<ewn::PlayerHandle*>)
	{
		if (instance.IsOfType(index, Nz::LuaType_Nil))
			*arg = nullptr;
		else
			*arg = static_cast<ewn::PlayerHandle*>(instance.CheckUserdata(index, "Player"))->GetObject();

		return 1;
	}

	inline unsigned int LuaImplReplyVal(const LuaState& state, ewn::Arena* arena, TypeTag<ewn::Arena*>)
	{
		state.PushInstance<ewn::Arena*>("Arena", arena);
		return 1;
	}

	inline unsigned int LuaImplReplyVal(const LuaState& state, ewn::Player* player, TypeTag<ewn::Player*>)
	{
		state.PushInstance<ewn::PlayerHandle>("Player", player);
		return 1;
	}

	inline unsigned int LuaImplReplyVal(const LuaState& state, const ewn::PlayerHandle& player, TypeTag<ewn::PlayerHandle>)
	{
		state.PushInstance<ewn::PlayerHandle>("Player", player);
		return 1;
	}
}