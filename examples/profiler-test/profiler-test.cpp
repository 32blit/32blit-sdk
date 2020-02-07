#include "profiler-test.hpp"
#include "graphics/color.hpp"
#include "engine/Profiler.hpp"
#include "engine/RunningAverage.hpp"
#include <cmath>

using namespace blit;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

Profiler g_profiler(50);
uint32_t g_uSize = 2;
uint32_t g_uSizeMax	= SCREEN_HEIGHT-1;
uint32_t g_uSizeMin	= 2;
uint32_t g_uSizeChange = 1;



#define RANDX random()%SCREEN_WIDTH
#define RANDY random()%SCREEN_HEIGHT
#define RANDC (int)(random()%256)


void init()
{
	set_screen_mode(ScreenMode::hires);
	g_profiler.SetDisplaySize(SCREEN_WIDTH, SCREEN_HEIGHT);
	g_profiler.SetRows(Profiler::DisplayType::dtText, 5);

	RunningAverage<float> test(8);

	for(int i = 1; i < 128; i++)
	{
		test.Add(i);
		volatile float fAvg = test.Average();
		printf("[%d] = %f\n\r", i, fAvg);
	}

}


void render(uint32_t time)
{
	static Point    ptMiddle(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);

	//static ProfilerProbe *pAwayRenderProbe = g_profiler.AddProbe("AwayFromRender");
	static ProfilerProbe *pRenderProbe = g_profiler.AddProbe("Render");
	static ProfilerProbe *pClearProbe = g_profiler.AddProbe("Clear");
	static ProfilerProbe *pRectProbe = g_profiler.AddProbe("Rectangle");
	static ProfilerProbe *pCircleProbe = g_profiler.AddProbe("Circle");


	//pAwayRenderProbe->StoreElapsedUs(true);

	pRenderProbe->Start();

	// clear screen
	pClearProbe->Start();
  screen.pen(RGBA(0, 0, 0, 255));
  screen.clear();
	pClearProbe->StoreElapsedUs();

  // draw rect
  pRectProbe->Start();
 	screen.pen(RGBA(0,0,255,255));
 	Point ptTl = Point(ptMiddle.x-(g_uSize/2), ptMiddle.y-(g_uSize/2));
 	Point ptBr = Point(ptMiddle.x+(g_uSize/2), ptMiddle.y+(g_uSize/2));
  screen.rectangle(Rect(ptTl, ptBr));
	pRectProbe->StoreElapsedUs();

  // draw circle
  pCircleProbe->Start();
 	screen.pen(RGBA(255,0,0,128));
  screen.circle(ptMiddle, (g_uSizeMax-g_uSize)/2);
	pCircleProbe->StoreElapsedUs();


	pRenderProbe->StoreElapsedUs();

  g_profiler.SetGraphTime(pRenderProbe->ElapsedMetrics().uMaxElapsedUs);
  g_profiler.DisplayProbeOverlay(Profiler::DisplayType::dtText, 1);
	g_profiler.LogProbes();

}

void update(uint32_t time)
{
	static ProfilerProbe *pUpdateProbe = g_profiler.AddProbe("Update");

	// Scoped profiler probes automatically call Start() and at end of scope call StoreElapsedUs()
	ScopedProfilerProbe scopedProbe(pUpdateProbe);
  // update size
  g_uSize+=g_uSizeChange;
  if(g_uSize >= g_uSizeMax)
  {
  	g_uSize -= (g_uSize-g_uSizeMax);
  	g_uSizeChange = - g_uSizeChange;
  }
  else
  {
    if(g_uSize <= g_uSizeMin)
    {
    	g_uSize += g_uSizeMin-g_uSize;
    	g_uSizeChange = - g_uSizeChange;
    }
  }

}
