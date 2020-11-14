#pragma once

// Profiler module
//
// A Profiler can have multiple ProfilerProbes
//
// A ProfilerProbe has a set of metrics measures in us: Min Value, Max Value, Current value, Average value
//
// The average value is calculated from a running average which can be set in the ProfilerProbe constructor along with the span
//
// Values can be logged to CDC via LogProbes()
//
// Values can be displayed as an overlay when called at the end of Render() using DisplayProbeOverlay()
//
// For examples of use and setup please see the profiler-test example.

#include <algorithm>
#include <string>
#include <vector>

#include "running_average.hpp"
#include "graphics/surface.hpp"


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

		Metrics()
		{
			clear();
		}

		void clear()
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


	ProfilerProbe(const char *pszName, uint32_t uRunningAverageSize = 0, uint32_t uRunningAverageSpan = 1) : m_pszName(pszName), m_uStartUs(0), m_metrics(), m_pRunningAverage(nullptr), m_uGraphTimeUs(20000)
	{
		if(uRunningAverageSize)
		{
			m_pRunningAverage = new RunningAverage<float>(uRunningAverageSize);
			if(uRunningAverageSpan == 0)
				m_uRunningAverageSpan = 1;
			else
				m_uRunningAverageSpan = uRunningAverageSpan;

			m_uRunningAverageSpanIndex = m_uRunningAverageSpan-1;
		}
	};

	void start();

	void clear()
	{
    	m_pRunningAverage->reset();
		m_metrics.clear();
		m_uStartUs = 0;
	}

	uint32_t store_elapsed_us(bool bRestart = false);

	const Metrics &elapsed_metrics()
	{
		return m_metrics;
	}

	const char *name()
	{
		return m_pszName;
	}

	const RunningAverage<float> *get_running_average()
	{
		return m_pRunningAverage;
	}

	void set_graph_time_us(uint32_t uGraphTimeUs)
	{
		m_uGraphTimeUs = uGraphTimeUs;
	}

	void set_graph_time_us_to_max()
	{
		m_uGraphTimeUs = m_metrics.uMaxElapsedUs;
	}

	uint32_t get_graph_time_us()
	{
		return m_uGraphTimeUs;
	}

private:
	const char 						*m_pszName;
	uint32_t							m_uStartUs;
	Metrics								m_metrics;
	RunningAverage<float> *m_pRunningAverage;
	uint32_t							m_uRunningAverageSpan;
	uint32_t							m_uRunningAverageSpanIndex;
	uint32_t							m_uGraphTimeUs;
};


class ScopedProfilerProbe
{
public:
	explicit ScopedProfilerProbe (ProfilerProbe  *pProbe)
	: m_pProbe(pProbe)
	{
		m_pProbe->start();
	}

	~ScopedProfilerProbe ()
	{
		m_pProbe->store_elapsed_us();
	}
	ScopedProfilerProbe ( ScopedProfilerProbe & ) = delete;
	void operator = ( ScopedProfilerProbe & ) = delete;

private:
	ProfilerProbe				*m_pProbe;
};


class Profiler
{
public:
	typedef std::vector<ProfilerProbe *>						ProfilerProbes;
	enum DisplayMetric {dmMin, dmCur, dmAvg, dmMax};


	struct GraphElement
	{
		GraphElement(): color(Pen(0,255,0)) {};

		bool			bDisplayLabel = false;
		bool			bDisplayGraph = false;
		Pen				color;
	};



	Profiler(uint32_t uRunningAverageSize = 0, uint32_t uRunningAverageSpan = 1);
	virtual ~Profiler();

	ProfilerProbe *add_probe(const char *pszName);
	ProfilerProbe *add_probe(const char *pszName,  uint32_t uRunningAverageSize, uint32_t uRunningAverageSpan=1);
	void					remove_probe(ProfilerProbe *pProbe);
	void					start_all_probes();
    void					clear_all_probes();

	void					log_probes();

	size_t        get_probe_count();
	size_t        get_page_count();

	void 					set_display_size(uint16_t uWidth, uint32_t uHeight);
	void					set_graph_time(uint32_t uTimeUs);
	void					set_rows(uint8_t uRows);
	void					set_alpha(uint8_t uAlpha);
	void					display_probe_overlay(uint8_t uPage);
	void					display_history(bool bDisplayHistory, Pen color = Pen(0,255,0));

	void 					setup_graph_element(DisplayMetric metric, bool bDisplayLabel, bool bDisplayGraph, Pen color);
	GraphElement  &get_graph_element(DisplayMetric metric);

private:
	static const char *g_pszMetricNames[];

	ProfilerProbes	m_probes;
	GraphElement    m_graphElements[dmMax+1];

	uint16_t				m_uWidth;
	uint16_t				m_uHeight;
	uint16_t				m_uRows;
	int32_t				  	m_uGraphTimeUs;
	uint32_t				m_uRunningAverageSize;
	uint32_t				m_uRunningAverageSpan;
	uint16_t				m_uRowHeight;
	uint16_t				m_uBorder;
	uint16_t				m_uHeaderSize;
	uint8_t					m_uAlpha;
	bool					m_bDisplayHistory;
	Pen						m_historyColor;

};
}; // namespace
