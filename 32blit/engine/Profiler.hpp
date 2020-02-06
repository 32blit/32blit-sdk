#pragma once

#include <string>
#include <vector>

extern void EnableUsTimer(void);
extern uint32_t GetUsTimer(void);
uint32_t GetMaxUsTimer(void);


class ProfilerProbe
{
public:
	struct Metrics
	{
		Metrics(void)
		{
			Clear();
		}

		void Clear(void)
		{
			uElapsedUs		= 0;
			uMinElapsedUs = UINT32_MAX;
			uMaxElapsedUs = 0;
		}

		uint32_t uElapsedUs;
		uint32_t uMinElapsedUs;
		uint32_t uMaxElapsedUs;
	};


	ProfilerProbe(const char *pszName) : m_pszName(pszName), m_uStartUs(0), m_metrics() {};

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
	const char 			*m_pszName;
	uint32_t	m_uStartUs;
	Metrics		m_metrics;
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

//	ScopedProfilerProbe ( ScopedProfilerProbe & );
//	void operator = ( ScopedProfilerProbe & );
};


class Profiler
{
public:
	typedef std::vector<ProfilerProbe *>						ProfilerProbes;
	typedef std::vector<ProfilerProbe *>::iterator	ProfilerProbesIter;

	Profiler();
	virtual ~Profiler();

	ProfilerProbe *AddProbe(const char *pszName);
	void					RemoveProbe(ProfilerProbe *pProbe);
	void					StartAllProbes(void);
	void					LogProbes(void);
	uint32_t			GetProbeCount(void);
	uint32_t			GetPageCount(void);
	void 					SetDisplaySize(uint16_t uWidth, uint32_t uHeight);
	void					DisplayProbeOverlay(uint8_t uPage);
	void					SetGraphTime(uint32_t uTimeUs);

private:
	ProfilerProbes	m_probes;
	uint16_t				m_uWidth;
	uint16_t				m_uHeight;
	uint16_t				m_uTextLines;
	uint32_t				m_uGraphTimeUs;
};

