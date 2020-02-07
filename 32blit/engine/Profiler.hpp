#pragma once

#include <string>
#include <vector>

#include "32blit.hpp"
#include "RunningAverage.hpp"

extern void EnableUsTimer(void);
extern uint32_t GetUsTimer(void);
uint32_t GetMaxUsTimer(void);


namespace blit
{
class ProfilerProbe
{
public:
	struct Metrics
	{
		uint32_t operator[] (std::size_t i)
		{
			if(i==0)
				return uMinElapsedUs;
			if(i==1)
				return uElapsedUs;
			if(i==2)
				return uAvgElapsedUs;
			if(i==3)
				return uMaxElapsedUs;

			return 0;
		}

		Metrics(void)
		{
			Clear();
		}

		void Clear(void)
		{
			uElapsedUs		= 0;
			uMinElapsedUs = UINT32_MAX;
			uMaxElapsedUs = 0;
			uAvgElapsedUs = 0;
		}

		uint32_t uElapsedUs;
		uint32_t uMinElapsedUs;
		uint32_t uMaxElapsedUs;
		uint32_t uAvgElapsedUs;
	};


	ProfilerProbe(const char *pszName, uint32_t uRunningAverageSize = 0) : m_pszName(pszName), m_uStartUs(0), m_metrics(), m_pRunningAverage(NULL)
	{
		if(uRunningAverageSize)
			m_pRunningAverage = new RunningAverage<float>(uRunningAverageSize);
	};

	void Start(void)
	{
		m_uStartUs = GetUsTimer();
	}

	void Clear(void)
	{
		m_metrics.Clear();
		m_uStartUs = 0;
	}

	uint32_t StoreElapsedUs(bool bRestart = false)
	{
		if(m_uStartUs)
		{
			uint32_t uCurrentUs = GetUsTimer();
			if(uCurrentUs >= m_uStartUs)
				m_metrics.uElapsedUs = uCurrentUs - m_uStartUs;
			else
				m_metrics.uElapsedUs = (GetMaxUsTimer() - m_uStartUs) + uCurrentUs;

			m_metrics.uMinElapsedUs = std::min(m_metrics.uMinElapsedUs, m_metrics.uElapsedUs);
			m_metrics.uMaxElapsedUs = std::max(m_metrics.uMaxElapsedUs, m_metrics.uElapsedUs);
			if(m_pRunningAverage)
			{
				m_pRunningAverage->Add((float)m_metrics.uElapsedUs);
				m_metrics.uAvgElapsedUs = m_pRunningAverage->Average();
			}
			else
				m_metrics.uAvgElapsedUs = m_metrics.uElapsedUs;
		}

	  if(bRestart)
	  	m_uStartUs = GetUsTimer();

	  return m_metrics.uElapsedUs;
	}

	const Metrics &ElapsedMetrics(void)
	{
		return m_metrics;
	}

	const char *Name(void)
	{
		return m_pszName;
	}

private:
	const char 						*m_pszName;
	uint32_t							m_uStartUs;
	Metrics								m_metrics;
	RunningAverage<float> *m_pRunningAverage;

};


class ScopedProfilerProbe
{
public:
	explicit ScopedProfilerProbe (ProfilerProbe  *pProbe)
	: m_pProbe(pProbe)
	{
		m_pProbe->Start();
	}

	~ScopedProfilerProbe ()
	{
		m_pProbe->StoreElapsedUs();
	}

private:
	ProfilerProbe				*m_pProbe;

	ScopedProfilerProbe ( ScopedProfilerProbe & );
	void operator = ( ScopedProfilerProbe & );
};


class Profiler
{
public:
	typedef std::vector<ProfilerProbe *>						ProfilerProbes;
	typedef std::vector<ProfilerProbe *>::iterator	ProfilerProbesIter;
	typedef enum {dtText, dtGraph}									DisplayType;
	typedef enum {dmMin, dmCur, dmAvg, dmMax}				DisplayMetric;


	struct GraphElement
	{
		GraphElement(void) :bDisplayLabel(true), bDisplayGraph(true), color(RGBA(0,255,0))	{};

		bool	bDisplayLabel;
		bool	bDisplayGraph;
		RGBA	color;
	};



	Profiler(uint32_t uRunningAverageSize = 0);
	virtual ~Profiler();

	ProfilerProbe *AddProbe(const char *pszName);
	void					RemoveProbe(ProfilerProbe *pProbe);
	void					StartAllProbes(void);

	void					LogProbes(void);

	uint32_t			GetProbeCount(void);
	uint32_t			GetPageCount(DisplayType displayType);

	void 					SetDisplaySize(uint16_t uWidth, uint32_t uHeight);
	void					SetGraphTime(uint32_t uTimeUs);
	void					SetRows(DisplayType displayType, uint8_t uRows);
	void					SetAlpha(uint8_t uAlpha);
	void					DisplayProbeOverlay(DisplayType displayType, uint8_t uPage);
	void					EnableMetric(DisplayMetric metric, bool bEnable);

private:
	static const char *g_pszMetricNames[];

	ProfilerProbes	m_probes;
	GraphElement    m_graphElements[dmMax+1];

	uint16_t				m_uWidth;
	uint16_t				m_uHeight;
	uint16_t				m_uTextRows;
	uint16_t				m_uGraphRows;
	uint32_t				m_uGraphTimeUs;
	uint32_t				m_uRunningAverageSize;
	uint16_t				m_uTextHeight;
	uint16_t				m_uGraphHeight;
	uint16_t				m_uBorder;
	uint16_t				m_uHeaderSize;
	uint8_t					m_uAlpha;

};
}; // namespace
