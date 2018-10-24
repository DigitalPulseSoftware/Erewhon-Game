// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_UTILS_DATETIME_HPP
#define EREWHON_SHARED_UTILS_DATETIME_HPP

namespace ewn
{
	struct Date
	{
		inline bool IsValid() const;

		static inline unsigned int GetMonthDayCount(unsigned int month, int year);
		static inline bool IsLeapYear(int year);

		int year;
		unsigned int month;
		unsigned int day;
	};

	struct Time
	{
		inline bool IsValid() const;

		unsigned int hour;
		unsigned int minutes;
		unsigned int seconds;
		unsigned int microseconds;
	};

	struct Datetime : Date, Time
	{
		inline bool IsValid() const;
	};
}

#include <Shared/Utils/Datetime.inl>

#endif // EREWHON_SHARED_UTILS_DATETIME_HPP
