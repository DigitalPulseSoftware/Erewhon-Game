// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/Scripting/LuaMathTypes.hpp>
#include <Nazara/Lua/LuaState.hpp>

namespace ewn
{
}

namespace Nz
{
	inline int LuaImplReplyVal(const LuaState& state, ewn::LuaQuaternion&& quat, TypeTag<ewn::LuaQuaternion>)
	{
		state.PushTable(0, 2);
			state.PushField("w", quat.w);
			state.PushField("x", quat.x);
			state.PushField("y", quat.y);
			state.PushField("z", quat.z);

		//TODO: Optimize metatable retrieval
		state.GetGlobal("Quaternion");

		state.SetMetatable(-2);
		return 1;
	}

	inline int LuaImplReplyVal(const LuaState& state, ewn::LuaVec2&& vec, TypeTag<ewn::LuaVec2>)
	{
		state.PushTable(0, 2);
			state.PushField("x", vec.x);
			state.PushField("y", vec.y);

		//TODO: Optimize metatable retrieval
		state.GetGlobal("Vec2");

		state.SetMetatable(-2);
		return 1;
	}

	inline int LuaImplReplyVal(const LuaState& state, ewn::LuaVec3&& vec, TypeTag<ewn::LuaVec3>)
	{
		state.PushTable(0, 3);
			state.PushField("x", vec.x);
			state.PushField("y", vec.y);
			state.PushField("z", vec.z);

		//TODO: Optimize metatable retrieval
		state.GetGlobal("Vec3");

		state.SetMetatable(-2);
		return 1;
	}
}
