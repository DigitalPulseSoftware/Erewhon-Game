// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Scripting/LuaTypes.hpp>
#include <Nazara/Lua/LuaState.hpp>
#include <optional>

namespace ewn
{
}

namespace Nz
{
	template<typename T>
	int LuaImplReplyVal(const LuaState& state, std::optional<T>&& opt, TypeTag<std::optional<T>>)
	{
		if (opt.has_value())
			return LuaImplReplyVal(state, std::move(opt.value()), TypeTag<T>());
		else
		{
			state.PushNil();
			return 1;
		}
	}
}
