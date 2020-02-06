#include <engine/Profiler.hpp>

Profiler::Profiler()
{
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

void Profiler::SetDisplayLines(uint8_t uDisplayLines)
{

}

void Profiler::DisplayProbeOverlay(uint8_t uPageSize)
{

}


