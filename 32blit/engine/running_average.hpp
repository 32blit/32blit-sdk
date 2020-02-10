#pragma once

#include <vector>

namespace blit
{
template<typename T>
class RunningAverage
{
public:
	typedef std::vector<T> Data;
	RunningAverage(std::size_t uSize) : m_uSize(uSize), m_uIndex(0), m_average(0), m_bFull(false) {};

	const T &operator[] (std::size_t i) const
	{
		std::size_t actualIndex = 0;

		if(m_bFull)
			actualIndex = ((m_uIndex + i) % m_uSize);
		else
			actualIndex = i;

		return m_data[actualIndex];
	}

	void Add(T value)
	{
		if(m_bFull)
		{
			T removed = m_data[m_uIndex];
			m_data[m_uIndex] = value;
			m_average = m_average - (removed / m_uSize) + (value / m_uSize);
		}
		else
		{
			m_data.push_back(value);
			m_average = 0;
			for(std::size_t iD = 0; iD <= m_uIndex; iD++)
				m_average += m_data[iD]/(m_uIndex+1);
		}

		m_uIndex = (m_uIndex+1)%m_uSize;
		if(!m_bFull)
			m_bFull = m_data.size() == m_uSize;

	}

	void AddAll(T value)
	{
		Reset();
	  while(m_uIndex < m_uSize)
			m_data[m_uIndex++] = value;
	}

	void Reset(void)
	{
		m_uSize 	= 0;
		m_uIndex 	= 0;
		m_average = 0;
		m_data.clear();

	}

	T Average(void)
	{
		return m_average;
	}

	std::size_t DataCount(void)
	{
		return m_data.size();
	}

	std::size_t Count(void) const
	{
		std::size_t uCount;

		if(m_bFull)
			uCount = m_uSize;
		else
			uCount = m_uIndex;

		return uCount;
	}

	std::size_t Size(void) const
	{
		return m_uSize;
	}


private:
	Data 				m_data;
	std::size_t m_uSize;
	std::size_t m_uIndex;
  T         	m_average;
  bool				m_bFull;
};
} // namespace

