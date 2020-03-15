#include <cinttypes>

#include "profiler.hpp"
#include "engine/api_private.hpp"
#include "engine/engine.hpp"
#include "graphics/color.hpp"
#include "graphics/font.hpp"

namespace blit
{

void ProfilerProbe::start()
{
	m_uStartUs = api.get_us_timer();
}

uint32_t ProfilerProbe::store_elapsed_us(bool bRestart)
{
	if(m_uStartUs)
	{
		uint32_t uCurrentUs = api.get_us_timer();
		if(uCurrentUs >= m_uStartUs)
			m_metrics.uElapsedUs = uCurrentUs - m_uStartUs;
		else
			m_metrics.uElapsedUs = (api.get_max_us_timer() - m_uStartUs) + uCurrentUs;

		m_metrics.uMinElapsedUs = std::min(m_metrics.uMinElapsedUs, m_metrics.uElapsedUs);
		m_metrics.uMaxElapsedUs = std::max(m_metrics.uMaxElapsedUs, m_metrics.uElapsedUs);
		if(m_pRunningAverage)
		{
			if(m_uRunningAverageSpanIndex == 0)
			{
				m_pRunningAverage->add((float)m_metrics.uElapsedUs);
				m_metrics.uAvgElapsedUs = m_pRunningAverage->average();
				m_uRunningAverageSpanIndex = m_uRunningAverageSpan-1;
			}
			else
				m_uRunningAverageSpanIndex--;
		}
		else
			m_metrics.uAvgElapsedUs = m_metrics.uElapsedUs;
	}

	if(bRestart)
	m_uStartUs = api.get_us_timer();

	return m_metrics.uElapsedUs;
}


const char *Profiler::g_pszMetricNames[4]= {"Min", "Cur", "Avg", "Max"};

Profiler::Profiler(uint32_t uRunningAverageSize, uint32_t uRunningAverageSpan ) : m_uGraphTimeUs(20000), m_uRunningAverageSize(uRunningAverageSize), m_uRunningAverageSpan(uRunningAverageSpan), m_uRowHeight(10), m_uBorder(5), m_uHeaderSize(15), m_uAlpha(160)
{
	api.enable_us_timer();

	// default to lowres
	set_display_size(160, 120);

	// default, just display cur, no bars, no history
	m_graphElements[dmCur].bDisplayLabel = true;
}

Profiler::~Profiler()
{
}



ProfilerProbe *Profiler::add_probe(const char *pszName)
{
	ProfilerProbe *pProbe = new ProfilerProbe(pszName, m_uRunningAverageSize, m_uRunningAverageSpan);
	m_probes.push_back(pProbe);

	return pProbe;
}

ProfilerProbe *Profiler::add_probe(const char *pszName,  uint32_t uRunningAverageSize, uint32_t uRunningAverageSpan)
{
	ProfilerProbe *pProbe = new ProfilerProbe(pszName, uRunningAverageSize, uRunningAverageSpan);
	m_probes.push_back(pProbe);

	return pProbe;
}

void Profiler::remove_probe(ProfilerProbe *pProbe)
{

}

void Profiler::start_all_probes()
{
	for(ProfilerProbesIter iP = m_probes.begin(); iP != m_probes.end(); iP++)
		(*iP)->start();
}

void Profiler::clear_all_probes()
{
	for(ProfilerProbesIter iP = m_probes.begin(); iP != m_probes.end(); iP++)
		(*iP)->clear();
}
void Profiler::log_probes()
{
	for(ProfilerProbesIter iP = m_probes.begin(); iP != m_probes.end(); iP++)
	{
		ProfilerProbe *pProbe = *(iP);
		const ProfilerProbe::Metrics &metrics = pProbe->elapsed_metrics();
		printf("%-16s %" PRIu32 ",\t%" PRIu32 ",\t%" PRIu32 ",\t%" PRIu32 "\n\r", pProbe->name(), metrics.uMinElapsedUs, metrics.uElapsedUs, metrics.uAvgElapsedUs, metrics. uMaxElapsedUs);
	}
	printf("\n\r");
}

uint32_t Profiler::get_probe_count()
{
	return m_probes.size();
}

uint32_t Profiler::get_page_count()
{
	uint32_t uPages = (m_probes.size() + m_uRows - 1) / m_uRows;
	return uPages;
}

void Profiler::set_display_size(uint16_t uWidth, uint32_t uHeight)
{
	m_uWidth  = uWidth;
	m_uHeight = uHeight-20;
	m_uRows  = uHeight/m_uRowHeight;
}


void Profiler::set_rows(uint8_t uRows)
{
	m_uRows = uRows;
	m_uRowHeight = m_uHeight / m_uRows;
}


void Profiler::set_graph_time(uint32_t uTimeUs)
{
	m_uGraphTimeUs = uTimeUs;
}

void Profiler::set_alpha(uint8_t uAlpha)
{
	m_uAlpha = uAlpha;
}

Profiler::GraphElement &Profiler::get_graph_element(DisplayMetric metric)
{
	return m_graphElements[metric];
}

void Profiler::setup_graph_element(DisplayMetric metric, bool bDisplayLabel, bool bDisplayGraph, Pen color)
{
	m_graphElements[metric].bDisplayLabel = bDisplayLabel;
	m_graphElements[metric].bDisplayGraph = bDisplayGraph;
	m_graphElements[metric].color = color;
}

void Profiler::display_history(bool bDisplayHistory, Pen color)
{
	m_bDisplayHistory = bDisplayHistory;
	m_historyColor = color;
}

void Profiler::display_probe_overlay(uint8_t uPage)
{
	if(uPage > 0)
	{
		char buffer[64];

		uint8_t uLabelCount = 0;
		for(uint8_t uM = dmMin; uM <= dmMax; uM++)
		{
			if(m_graphElements[uM].bDisplayLabel)
				uLabelCount++;
		}


		uint16_t uUseWidth = m_uWidth-(m_uBorder*2);
		uint16_t uNameWidth = uUseWidth / 3;
		uint16_t uMetricWidth = (uNameWidth *2)/uLabelCount;
		uint16_t uNameX = m_uBorder;
		uint16_t uMetricX  = uNameX + uNameWidth;

		uint16_t uMaxPage = get_page_count();
		if(uPage > uMaxPage)
			uPage = uMaxPage;

		// display header
		screen.pen = Pen(255, 255, 255, m_uAlpha);
		sprintf(buffer, "%" PRIu32 " (%u/%u)", m_uGraphTimeUs, uPage, uMaxPage);
		screen.text(buffer, minimal_font, Point(m_uBorder, m_uBorder));

		// labels
		for(uint8_t uM = dmMin; uM <= dmMax; uM++)
		{
			if(m_graphElements[uM].bDisplayLabel)
			{
				screen.text(g_pszMetricNames[uM], minimal_font, Point(uMetricX, m_uBorder));
				uMetricX+=uMetricWidth;
			}
		}




		uint16_t uStartProbe = (uPage-1) * m_uRows;


		uint16_t uY = m_uHeaderSize;
		if(uStartProbe < get_probe_count())
		{
			uint8_t uBarCount = 0;
			for(uint8_t uM = dmMin; uM <= dmMax; uM++)
			{
				if(m_graphElements[uM].bDisplayGraph)
					uBarCount++;
			}

			uint16_t uBarHeight = (m_uRowHeight/uBarCount);


			for(ProfilerProbesIter iP = m_probes.begin() + uStartProbe; iP != m_probes.begin() + uStartProbe + m_uRows && iP != m_probes.end(); iP++)
			{
				ProfilerProbe *pProbe = (*iP);
				ProfilerProbe::Metrics metrics = pProbe->elapsed_metrics();

				uint32_t uUseGraphTimeUs;
				if(m_uGraphTimeUs != 0)
					uUseGraphTimeUs = m_uGraphTimeUs;
				else
					uUseGraphTimeUs = pProbe->get_graph_time_us();

				screen.pen =Pen(255, 255, 255, m_uAlpha);
				screen.text(pProbe->name(), minimal_font, Rect(m_uBorder, uY, uNameWidth, m_uRowHeight), true, TextAlign::center_v);

				uMetricX  = uNameX + uNameWidth;
				for(uint8_t uM = dmMin; uM <= dmMax; uM++)
				{
					screen.pen = Pen(255, 255, 255, m_uAlpha);
					if(m_graphElements[uM].bDisplayLabel)
					{
						sprintf(buffer, "%" PRIu32, metrics[uM]);
						screen.text(buffer, minimal_font, Rect(uMetricX, uY, uMetricWidth, m_uRowHeight), true, TextAlign::center_v);
						uMetricX+=uMetricWidth;
					}

					if(m_graphElements[uM].bDisplayGraph)
					{
						m_graphElements[uM].color.a = m_uAlpha/2;
						screen.pen = m_graphElements[uM].color;

						uint16_t uBarWidth = (float)uUseWidth * ((float)metrics[uM] / (float)uUseGraphTimeUs);
						if(uBarWidth < 1)
							uBarWidth = 1;
						screen.rectangle(Rect(m_uBorder, uY + (uM * uBarHeight), uBarWidth, uBarHeight-1));
					}
				}

				screen.pen = Pen(255, 255, 255, m_uAlpha/2);
				screen.line(Point(m_uBorder, uY), Point(uUseWidth + m_uBorder, uY));

				if(m_bDisplayHistory)
				{
					screen.pen = Pen(0, 255, 0, m_uAlpha);

					const RunningAverage<float> *pRunningAverage = pProbe->get_running_average();
					if(pRunningAverage)
					{
						const std::size_t uDataPoints = pRunningAverage->count();
						const std::size_t uSize       = pRunningAverage->size();

						float fXInc = (float)uUseWidth / uSize;
						float fX = m_uBorder+(uUseWidth) - (fXInc*uDataPoints);
						uint32_t uLastX = 0;
						uint32_t uLastDataY = 0;

						for(std::size_t uDp = 0; uDp < uDataPoints; uDp++)
						{
							std::size_t uUseDp = uDp;
							if(fXInc < 1.0f)
								uUseDp = (float)uDp * (1.0f/fXInc);

							const float fDataPoint = (*pRunningAverage)[uUseDp];
							uint32_t uDataY = m_uRowHeight * (fDataPoint/ (float)uUseGraphTimeUs);
							if(uDataY > m_uRowHeight)
								uDataY = m_uRowHeight;

							if(uDp != 0)
								screen.line(Point(uLastX, (uY + m_uRowHeight) - uLastDataY), Point(fX, (uY + m_uRowHeight) - uDataY));

							uLastDataY = uDataY;
							uLastX = fX;

							if(fXInc > 1.0f)
								fX+=fXInc;
							else
								fX++;
						}
					}
				}

				uY+=m_uRowHeight;
			}
			screen.pen = Pen(255, 255, 255, m_uAlpha/2);
			screen.line(Point(m_uBorder, uY), Point(m_uWidth + m_uBorder, uY));
		}
	}
}
} // namespace


