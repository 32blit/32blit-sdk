#include <engine/Profiler.hpp>
#include "graphics/color.hpp"

namespace blit
{


const char *Profiler::g_pszMetricNames[4]= {"Min", "Cur", "Avg", "Max"};

Profiler::Profiler(uint32_t uRunningAverageSize) : m_uGraphTimeUs(20000), m_uRunningAverageSize(uRunningAverageSize), m_uTextHeight(10), m_uBorder(5), m_uHeaderSize(20), m_uAlpha(160)
{
	// default to lowres
	SetDisplaySize(160, 120);

}

Profiler::~Profiler()
{
}



ProfilerProbe *Profiler::AddProbe(const char *pszName)
{
	ProfilerProbe *pProbe = new ProfilerProbe(pszName, m_uRunningAverageSize);
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
	for(ProfilerProbesIter iP = m_probes.begin(); iP != m_probes.end(); iP++)
	{
		ProfilerProbe *pProbe = *(iP);
		const ProfilerProbe::Metrics &metrics = pProbe->ElapsedMetrics();
		printf("%-16s %lu,\t%lu,\t%lu\n\r", pProbe->Name(), metrics.uMinElapsedUs, metrics.uElapsedUs, metrics. uMaxElapsedUs);
	}
}

uint32_t Profiler::GetProbeCount(void)
{
	return m_probes.size();
}

uint32_t Profiler::GetPageCount(DisplayType displayType)
{
	uint32_t uPages = 0;
	switch(displayType)
	{
		case dtText :
			uPages = (GetProbeCount()/m_uTextRows)+1;
		break;

		case dtGraph :
			uPages = (GetProbeCount()/m_uGraphRows)+1;
		break;

	}

	return uPages;
}

void Profiler::SetDisplaySize(uint16_t uWidth, uint32_t uHeight)
{
	m_uWidth  = uWidth;
	m_uHeight = uHeight-20;
	m_uTextRows  = uHeight/m_uTextHeight;
	m_uGraphRows = m_uTextRows / 5;
}


void Profiler::SetRows(DisplayType displayType, uint8_t uRows)
{
	switch(displayType)
	{
		case dtText :
			m_uTextRows = uRows;
			m_uTextHeight = m_uHeight / m_uTextRows;
		break;

		case dtGraph :
			m_uGraphRows = uRows;
			m_uGraphHeight = m_uHeight / m_uGraphRows;
		break;

	}
}


void Profiler::SetGraphTime(uint32_t uTimeUs)
{
	m_uGraphTimeUs = uTimeUs;
}

void Profiler::DisplayProbeOverlay(DisplayType displayType, uint8_t uPage)
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


		//screen.pen(RGBA(30, 30, 50, m_uAlpha));
		//screen.clear();




		// display header
		screen.pen(RGBA(255, 255, 255, m_uAlpha));
		sprintf(buffer, "%luus", m_uGraphTimeUs);
		screen.text(buffer, &minimal_font[0][0], Point(5, 5));

		// labels
		for(uint8_t uM = dmMin; uM <= dmMax; uM++)
		{
			if(m_graphElements[uM].bDisplayLabel)
			{
				screen.text(g_pszMetricNames[uM], &minimal_font[0][0], Point(uMetricX, 5));
				uMetricX+=uMetricWidth;
			}
		}

		// Horizontal Line
		screen.pen(RGBA(255, 255, 255, m_uAlpha));
		screen.rectangle(Rect(0, m_uHeaderSize - 5, m_uWidth, 1));



		uint16_t uMaxPage = GetPageCount(displayType);
		if(uPage > uMaxPage)
			uPage = uMaxPage;

		uint16_t uStartProbe = (uPage-1) * m_uTextRows;


		uint16_t uY = m_uHeaderSize;
		if(uStartProbe < GetProbeCount())
		{
			uint8_t uBarCount = 0;
			for(uint8_t uM = dmMin; uM <= dmMax; uM++)
			{
				if(m_graphElements[uM].bDisplayGraph)
					uBarCount++;
			}

			uint16_t uBarHeight = (m_uTextHeight/uBarCount) -1;


			for(ProfilerProbesIter iP = m_probes.begin() + uStartProbe; iP != m_probes.begin()+m_uTextRows && iP != m_probes.end(); iP++)
			{
				ProfilerProbe *pProbe = (*iP);
				ProfilerProbe::Metrics metrics = pProbe->ElapsedMetrics();

				screen.pen(RGBA(255, 255, 255, m_uAlpha));
				screen.text(pProbe->Name(), &minimal_font[0][0], Rect(5, uY, uNameWidth, m_uTextHeight), true, TextAlign::center_v);


				uMetricX  = uNameX + uNameWidth;
				for(uint8_t uM = dmMin; uM <= dmMax; uM++)
				{
					screen.pen(RGBA(255, 255, 255, m_uAlpha));
					if(m_graphElements[uM].bDisplayLabel)
					{
						sprintf(buffer, "%lu", metrics[uM]);
						screen.text(buffer, &minimal_font[0][0], Rect(uMetricX, uY, uMetricWidth, m_uTextHeight), true, TextAlign::center_v);
						uMetricX+=uMetricWidth;
					}

					if(m_graphElements[uM].bDisplayGraph)
					{
						m_graphElements[uM].color.a = m_uAlpha/2;
						screen.pen(m_graphElements[uM].color);
						uint16_t uBarWidth = (float)uUseWidth * ((float)metrics[uM] / (float)m_uGraphTimeUs);
						if(uBarWidth < 1)
							uBarWidth = 1;
						screen.rectangle(Rect(5, uY + (uM * uBarHeight), uBarWidth, uBarHeight));
					}
				}
				uY+=m_uTextHeight;
			}
		}
	}
}
} // namespace

