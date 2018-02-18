// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_UTILS_PIDCONTROLLER_HPP
#define EREWHON_SHARED_UTILS_PIDCONTROLLER_HPP

namespace ewn
{
	template<typename T>
	class PidController
	{
		public:
			PidController(float p, float i, float d);

			T Update(const T& currentError, float elapsedTime);

		private:
			T m_lastError;
			T m_integral;
			float m_dFactor;
			float m_iFactor;
			float m_pFactor;
	};
}

#include <Shared/Utils/PidController.inl>

#endif // EREWHON_SHARED_UTILS_PIDCONTROLLER_HPP
