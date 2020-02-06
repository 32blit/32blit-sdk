#include "profiler-test.hpp"
#include "graphics/color.hpp"
#include "engine/Profiler.hpp"

#include <cmath>

using namespace blit;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

Profiler	g_profiler;


#define RANDX random()%SCREEN_WIDTH
#define RANDY random()%SCREEN_HEIGHT
#define RANDC (int)(random()%256)


void init()
{
	set_screen_mode(ScreenMode::hires);
}


void render(uint32_t time)
{
	static ProfilerProbe *pAwayRenderProbe = g_profiler.AddProbe("AwayFromRender");
	static ProfilerProbe *pRenderProbe = g_profiler.AddProbe("Render");
	static ProfilerProbe *pClearProbe = g_profiler.AddProbe("Clear");
	static ProfilerProbe *pRectProbe = g_profiler.AddProbe("Rectangle");
	static ProfilerProbe *pCircleProbe = g_profiler.AddProbe("Circle");

	static uint32_t uSize       = 2;
	static uint32_t uSizeChange = 1;
	static uint32_t uSizeMax 		= SCREEN_HEIGHT-1;
	static uint32_t uSizeMin 		= 2;
	static Point    ptMiddle(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);

	pAwayRenderProbe->StoreElapsedUs(true);

	pRenderProbe->Start();

	// clear screen
	pClearProbe->Start();
  screen.pen(RGBA(0, 0, 0, 255));
  screen.clear();
	pClearProbe->StoreElapsedUs();

  // draw rect
  pRectProbe->Start();
 	screen.pen(RGBA(0,0,255,255));
 	Point ptTl = Point(ptMiddle.x-(uSize/2), ptMiddle.y-(uSize/2));
 	Point ptBr = Point(ptMiddle.x+(uSize/2), ptMiddle.y+(uSize/2));
  screen.rectangle(Rect(ptTl, ptBr));
	pRectProbe->StoreElapsedUs();

  // draw circle
  pCircleProbe->Start();
 	screen.pen(RGBA(255,0,0,128));
  screen.circle(ptMiddle, (uSizeMax-uSize)/2);
	pCircleProbe->StoreElapsedUs();


	pRenderProbe->StoreElapsedUs();

  // update size
  uSize+=uSizeChange;
  if(uSize >= uSizeMax)
  {
  	uSize -= (uSize-uSizeMax);
  	uSizeChange = - uSizeChange;
  }
  else
  {
    if(uSize <= uSizeMin)
    {
    	uSize += uSizeMin-uSize;
    	uSizeChange = - uSizeChange;
    }
  }


	g_profiler.LogProbes();

}

void update(uint32_t time)
{
	static ProfilerProbe *pUpdateProbe = g_profiler.AddProbe("Update");

	// code here
	ScopedProfilerProbe scopedProbe(pUpdateProbe);

}
