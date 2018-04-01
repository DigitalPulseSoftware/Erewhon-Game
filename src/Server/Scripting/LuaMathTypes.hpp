// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SCRIPTING_MATHTYPES_HPP
#define EREWHON_SCRIPTING_MATHTYPES_HPP

#include <Nazara/Math/Quaternion.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <Nazara/Math/Vector3.hpp>

namespace ewn
{
	class LuaQuaternion : public Nz::Quaternionf
	{
		public:
			using Quaternion::Quaternion;

			using Quaternion::operator=;
	};

	class LuaVec2 : public Nz::Vector2f
	{
		public:
			using Vector2::Vector2;

			using Vector2::operator=;
	};

	class LuaVec3 : public Nz::Vector3f
	{
		public:
			using Vector3::Vector3;

			using Vector3::operator=;
	};
}

#include <Server/Scripting/LuaMathTypes.inl>

#endif // EREWHON_SCRIPTING_MATHTYPES_HPP
