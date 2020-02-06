#include <engine/Profiler.hpp>
#include "graphics/color.hpp"
#include "32blit.hpp"
using namespace blit;


Profiler::Profiler() : m_uGraphTimeUs(20000)
{
	// default to lowres
	SetDisplaySize(160, 120);
}

Profiler::~Profiler()
{
}



ProfilerProbe *Profiler::AddProbe(const char *pszName)
{
	ProfilerProbe *pProbe = new ProfilerProbe(pszName);
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

uint32_t Profiler::GetPageCount(void)
{
	return (m_probes.size()/GetProbeCount())+1;
}

void Profiler::SetDisplaySize(uint16_t uWidth, uint32_t uHeight)
{
	m_uWidth = uWidth;
	m_uHeight = uHeight;
	m_uTextLines = (uHeight-20)/10;
}

void Profiler::SetGraphTime(uint32_t uTimeUs)
{
	m_uGraphTimeUs = uTimeUs;
}

void Profiler::DisplayProbeOverlay(uint8_t uPage)
{
	if(uPage > 0)
	{
		char buffer[64];

		uint16_t uUseWidth = m_uWidth-10;
		uint16_t uNameWidth = uUseWidth / 3;
		uint16_t uMetricWidth = (uNameWidth *2)/3;
		uint16_t uNameX = 5;
		uint16_t uMinX  = uNameX + uNameWidth;
		uint16_t uCurX  = uMinX + uMetricWidth;;
		uint16_t uMaxX  = uCurX + uMetricWidth;;


		// display header
		const RGBA graphColor = RGBA(0,255,0,200);

		screen.pen(RGBA(30, 30, 50, 200));
		screen.clear();

		screen.pen(RGBA(255, 255, 255));

		sprintf(buffer, "Profiler(%luus)", m_uGraphTimeUs);
		screen.text(buffer, &minimal_font[0][0], Point(5, 5));

		screen.text("Min", &minimal_font[0][0], Point(uMinX, 5));
		screen.text("Cur", &minimal_font[0][0], Point(uCurX, 5));
		screen.text("Max", &minimal_font[0][0], Point(uMaxX, 5));

		// Horizontal Line
		screen.pen(RGBA(255, 255, 255));
		screen.rectangle(Rect(0, 15, m_uWidth, 1));


		uint16_t uMaxPage = GetPageCount();
		if(uPage > uMaxPage)
			uPage = uMaxPage;

		uint16_t uStartProbe = (uPage-1) * m_uTextLines;

		uint16_t uY = 20;
		if(uStartProbe < GetProbeCount())
		{

			for(ProfilerProbesIter iP = m_probes.begin() + uStartProbe; iP != m_probes.end(); iP++)
			{
				ProfilerProbe *pProbe = (*iP);
				ProfilerProbe::Metrics metrics = pProbe->ElapsedMetrics();


				screen.pen(RGBA(255, 255, 255));
				screen.text(pProbe->Name(), &minimal_font[0][0], Rect(5, uY, uNameWidth, 10));


				sprintf(buffer, "%lu", metrics.uMinElapsedUs);
				screen.text(buffer, &minimal_font[0][0], Rect(uMinX, uY, uMetricWidth, 10));

				sprintf(buffer, "%lu", metrics.uElapsedUs);
				screen.text(buffer, &minimal_font[0][0], Rect(uCurX, uY, uMetricWidth, 10));

				sprintf(buffer, "%lu", metrics.uMaxElapsedUs);
				screen.text(buffer, &minimal_font[0][0], Rect(uMaxX, uY, uMetricWidth, 10));

				screen.pen( RGBA(0,255,0,100));
				uint16_t uBarWidth = (float)uUseWidth * ((float)metrics.uElapsedUs / (float)m_uGraphTimeUs);
				screen.rectangle(Rect(5, uY-2, uBarWidth, 10));

				uY+=10;
			}
		}
	}
}


