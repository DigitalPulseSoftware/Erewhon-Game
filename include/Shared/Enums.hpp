// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_ENUMS_HPP
#define EREWHON_SHARED_ENUMS_HPP

#include <Nazara/Prerequisites.hpp>
#include <Nazara/Core/Flags.hpp>

namespace ewn
{
	enum class BotMessageType : Nz::UInt8
	{
		Error,
		Warning,
		Info
	};

	enum class CreateFleetFailureReason : Nz::UInt8
	{
		AlreadyExists,
		ServerError
	};

	enum class CreateSpaceshipFailureReason : Nz::UInt8
	{
		AlreadyExists,
		ServerError
	};

	enum class DeleteFleetFailureReason : Nz::UInt8
	{
		MustHaveAtLeastOne,
		NotFound,
		ServerError
	};

	enum class DeleteSpaceshipFailureReason : Nz::UInt8
	{
		MustHaveAtLeastOne,
		NotFound,
		ServerError
	};

	enum class LoginFailureReason : Nz::UInt8
	{
		AccountNotFound,
		InvalidToken,
		PasswordMismatch,
		ServerError
	};

	enum class ModuleType : Nz::UInt8
	{
		// <!> Do not preserve alphabetical order, put new items at the end (id are importants)
		Engine,
		Navigation,
		Radar,
		Weapon,
		Communications,

		Max = Communications
	};

	enum class RegisterFailureReason : Nz::UInt8
	{
		EmailAlreadyTaken,
		LoginAlreadyTaken,
		ServerError
	};

	enum class SpaceshipQueryInfo : Nz::UInt8
	{
		Code,
		HullModelPath,
		Modules,
		Name,

		Max = Name
	};

	enum class UpdateFleetFailureReason : Nz::UInt8
	{
		NotFound,
		ServerError
	};

	enum class UpdateSpaceshipFailureReason : Nz::UInt8
	{
		NotFound,
		ServerError
	};

	const char* EnumToString(ModuleType moduleType);
}

namespace Nz
{
	template<>
	struct EnumAsFlags<ewn::SpaceshipQueryInfo>
	{
		static constexpr ewn::SpaceshipQueryInfo max = ewn::SpaceshipQueryInfo::Max;
	};
}

namespace ewn
{
	using SpaceshipQueryInfoFlags = Nz::Flags<SpaceshipQueryInfo>;
}

#endif // EREWHON_SHARED_ENUMS_HPP
