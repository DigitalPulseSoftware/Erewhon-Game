// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Utils/Datetime.hpp>
#include <cassert>

namespace ewn
{
	inline bool Date::IsValid() const
	{
		if (month < 1 || month > 12)
			return false;

		if (day < 1 || day > GetMonthDayCount(month, year))
			return false;

		return true;
	}

	inline unsigned int Date::GetMonthDayCount(unsigned int month, int year)
	{
		assert(month >= 1 && month <= 11);

		switch (month)
		{
			case 1:
			case 3:
			case 5:
			case 7:
			case 8:
			case 10:
			case 12:
				return 31;

			case 4:
			case 6:
			case 9:
			case 11:
				return 30;

			case 2:
				return (IsLeapYear(year) ? 29 : 28);
		}

		assert(false);
		return false;
	}

	inline bool Date::IsLeapYear(int year)
	{
		if (year % 4 == 0 && year % 100 != 0)
			return true;
		else if (year % 400 == 0)
			return true;

		return false;
	}

	inline bool Time::IsValid() const
	{
		if (hour > 23 || minutes > 59 || seconds > 59 || microseconds > 999'999)
			return false;

		return true;
	}

	inline bool Datetime::IsValid() const
	{
		return Date::IsValid() && Time::IsValid();
	}
}
