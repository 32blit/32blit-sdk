#pragma once

#include <vector>

namespace blit
{
template<typename T>
class RunningAverage
{
public:
  using Data = std::vector<T>;
	RunningAverage(std::size_t uSize) : m_uSize(uSize), m_uIndex(0), m_average(0), m_bFull(false) {
    m_data.reserve(uSize);
  };

	const T &operator[] (std::size_t i) const
	{
		std::size_t actualIndex = 0;

		if(m_bFull)
			actualIndex = ((m_uIndex + i) % m_uSize);
		else
			actualIndex = i;

		return m_data[actualIndex];
	}

	void add(T value)
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

	void add_all(T value)
	{
		reset();
	  while(m_uIndex < m_uSize)
			m_data[m_uIndex++] = value;
	}

	void reset()
	{
		m_uIndex 	= 0;
		m_average = 0;
    	m_bFull   = false;
		m_data.clear();

	}

	T average()
	{
		return m_average;
	}

	std::size_t data_count()
	{
		return m_data.size();
	}

	std::size_t count() const
	{
		std::size_t uCount;

		if(m_bFull)
			uCount = m_uSize;
		else
			uCount = m_uIndex;

		return uCount;
	}

	std::size_t size() const
	{
		return m_uSize;
	}


private:
	Data 				m_data;
	const std::size_t m_uSize;
	std::size_t m_uIndex;
  T         	m_average;
  bool				m_bFull;
};
} // namespace

