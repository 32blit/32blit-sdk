#include <cinttypes>

#include <engine/Profiler.hpp>
#include "graphics/color.hpp"

namespace blit
{


const char *Profiler::g_pszMetricNames[4]= {"Min", "Cur", "Avg", "Max"};

Profiler::Profiler(uint32_t uRunningAverageSize, uint32_t uRunningAverageSpan ) : m_uGraphTimeUs(20000), m_uRunningAverageSize(uRunningAverageSize), m_uRunningAverageSpan(uRunningAverageSpan), m_uRowHeight(10), m_uBorder(5), m_uHeaderSize(15), m_uAlpha(160)
{
	EnableUsTimer();

	// default to lowres
	SetDisplaySize(160, 120);

	// default, just display cur, no bars, no history
	m_graphElements[dmCur].bDisplayLabel = true;
}

Profiler::~Profiler()
{
}



ProfilerProbe *Profiler::AddProbe(const char *pszName)
{
	ProfilerProbe *pProbe = new ProfilerProbe(pszName, m_uRunningAverageSize, m_uRunningAverageSpan);
	m_probes.push_back(pProbe);

	return pProbe;
}

ProfilerProbe *Profiler::AddProbe(const char *pszName,  uint32_t uRunningAverageSize, uint32_t uRunningAverageSpan)
{
	ProfilerProbe *pProbe = new ProfilerProbe(pszName, uRunningAverageSize, uRunningAverageSpan);
	m_probes.push_back(pProbe);

	return pProbe;
}

void Profiler::RemoveProbe(ProfilerProbe *pProbe)
{

}

void Profiler::StartAllProbes(void)
{
	for(ProfilerProbesIter iP = m_probes.begin(); iP != m_probes.end(); iP++)
		(*iP)->Start();
}

void Profiler::LogProbes(void)
{
	printf("\n\r");
	for(ProfilerProbesIter iP = m_probes.begin(); iP != m_probes.end(); iP++)
	{
		ProfilerProbe *pProbe = *(iP);
		const ProfilerProbe::Metrics &metrics = pProbe->ElapsedMetrics();
		printf("%-16s %" PRIu32 ",\t%" PRIu32 ",\t%" PRIu32 ",\t%" PRIu32 "\n\r", pProbe->Name(), metrics.uMinElapsedUs, metrics.uElapsedUs, metrics.uAvgElapsedUs, metrics. uMaxElapsedUs);
	}

}

uint32_t Profiler::GetProbeCount(void)
{
	return m_probes.size();
}

uint32_t Profiler::GetPageCount()
{
	uint32_t uPages = (m_probes.size() + m_uRows - 1) / m_uRows;
	return uPages;
}

void Profiler::SetDisplaySize(uint16_t uWidth, uint32_t uHeight)
{
	m_uWidth  = uWidth;
	m_uHeight = uHeight-20;
	m_uRows  = uHeight/m_uRowHeight;
}


void Profiler::SetRows(uint8_t uRows)
{
	m_uRows = uRows;
	m_uRowHeight = m_uHeight / m_uRows;
}


void Profiler::SetGraphTime(uint32_t uTimeUs)
{
	m_uGraphTimeUs = uTimeUs;
}

void Profiler::SetAlpha(uint8_t uAlpha)
{
	m_uAlpha = uAlpha;
}

Profiler::GraphElement &Profiler::GetGraphElement(DisplayMetric metric)
{
	return m_graphElements[metric];
}

void Profiler::SetupGraphElement(DisplayMetric metric, bool bDisplayLabel, bool bDisplayGraph, RGBA color)
{
	m_graphElements[metric].bDisplayLabel = bDisplayLabel;
	m_graphElements[metric].bDisplayGraph = bDisplayGraph;
	m_graphElements[metric].color = color;
}

void Profiler::DisplayHistory(bool bDisplayHistory, RGBA color)
{
	m_bDisplayHistory = bDisplayHistory;
	m_historyColor = color;
}

void Profiler::DisplayProbeOverlay(uint8_t uPage)
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

		uint16_t uMaxPage = GetPageCount();
		if(uPage > uMaxPage)
			uPage = uMaxPage;

		// display header
		screen.pen(RGBA(255, 255, 255, m_uAlpha));
		sprintf(buffer, "%" PRIu32 " (%u/%u)", m_uGraphTimeUs, uPage, uMaxPage);
		screen.text(buffer, &minimal_font[0][0], Point(m_uBorder, m_uBorder));

		// labels
		for(uint8_t uM = dmMin; uM <= dmMax; uM++)
		{
			if(m_graphElements[uM].bDisplayLabel)
			{
				screen.text(g_pszMetricNames[uM], &minimal_font[0][0], Point(uMetricX, m_uBorder));
				uMetricX+=uMetricWidth;
			}
		}




		uint16_t uStartProbe = (uPage-1) * m_uRows;


		uint16_t uY = m_uHeaderSize;
		if(uStartProbe < GetProbeCount())
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
				ProfilerProbe::Metrics metrics = pProbe->ElapsedMetrics();

				uint32_t uUseGraphTimeUs;
				if(m_uGraphTimeUs != 0)
					uUseGraphTimeUs = m_uGraphTimeUs;
				else
					uUseGraphTimeUs = pProbe->GetGraphTimeUs();

				screen.pen(RGBA(255, 255, 255, m_uAlpha));
				screen.text(pProbe->Name(), &minimal_font[0][0], Rect(m_uBorder, uY, uNameWidth, m_uRowHeight), true, TextAlign::center_v);

				uMetricX  = uNameX + uNameWidth;
				for(uint8_t uM = dmMin; uM <= dmMax; uM++)
				{
					screen.pen(RGBA(255, 255, 255, m_uAlpha));
					if(m_graphElements[uM].bDisplayLabel)
					{
						sprintf(buffer, "%" PRIu32, metrics[uM]);
						screen.text(buffer, &minimal_font[0][0], Rect(uMetricX, uY, uMetricWidth, m_uRowHeight), true, TextAlign::center_v);
						uMetricX+=uMetricWidth;
					}

					if(m_graphElements[uM].bDisplayGraph)
					{
						m_graphElements[uM].color.a = m_uAlpha/2;
						screen.pen(m_graphElements[uM].color);

						uint16_t uBarWidth = (float)uUseWidth * ((float)metrics[uM] / (float)uUseGraphTimeUs);
						if(uBarWidth < 1)
							uBarWidth = 1;
						screen.rectangle(Rect(m_uBorder, uY + (uM * uBarHeight), uBarWidth, uBarHeight-1));
					}
				}

				screen.pen(RGBA(255, 255, 255, m_uAlpha/2));
				screen.line(Point(m_uBorder, uY), Point(uUseWidth + m_uBorder, uY));

				if(m_bDisplayHistory)
				{
					screen.pen(RGBA(0, 255, 0, m_uAlpha));

					const RunningAverage<float> *pRunningAverage = pProbe->GetRunningAverage();
					if(pRunningAverage)
					{
						const std::size_t uDataPoints = pRunningAverage->Count();
						const std::size_t uSize       = pRunningAverage->Size();

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
			screen.pen(RGBA(255, 255, 255, m_uAlpha/2));
			screen.line(Point(m_uBorder, uY), Point(m_uWidth + m_uBorder, uY));
		}
	}
}
} // namespace


