// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Utils/PidController.hpp>

namespace ewn
{
	template<typename T>
	inline PidController<T>::PidController(float p, float i, float d) :
	m_lastError(0),
	m_integral(0),
	m_dFactor(d),
	m_iFactor(i),
	m_pFactor(p)
	{
	}

	template<typename T>
	inline T PidController<T>::Update(const T& currentError, float elapsedTime)
	{
		m_integral += currentError * elapsedTime;
		T deriv = (currentError - m_lastError) / elapsedTime;
		m_lastError = currentError;

		return currentError * m_pFactor + m_integral * m_iFactor + deriv * m_dFactor;
	}
}
