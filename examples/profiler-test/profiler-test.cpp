// Example showing how to use the profiler
//
// Button			Function
// =====================================================
// A					Enable/Disable Bar Graphs
// B					Enable/Disable Text Values
// X					Enable/Disable History Graph
// Y					Switch graph time between global, set by maximum render time
//						and individual times, set by each probles maximum time
// UP					Reduce rows displayed on page
// DOWN				Increase rows displayed on page
// LEFT				Back Page
// RIGHT			Next Page


#include "profiler-test.hpp"
#include "graphics/color.hpp"
#include "engine/Profiler.hpp"
#include "engine/RunningAverage.hpp"
#include <cmath>

using namespace blit;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

Profiler 			g_profiler; // global uRunningAverageSize and uRunningAverageSpan could be set here.
ProfilerProbe *g_pRenderProbe;
ProfilerProbe *g_pClearProbe;
ProfilerProbe *g_pRectProbe;
ProfilerProbe *g_pCircleProbe;
ProfilerProbe *g_pUpdateProbe;


uint32_t g_uSize = 2;
uint32_t g_uSizeMax	= SCREEN_HEIGHT-1;
uint32_t g_uSizeMin	= 2;
uint32_t g_uSizeChange = 1;

bool g_bGraphEnabled   = true;
bool g_bLabelsEnabled  = true;
bool g_bDisplayHistory = true;
bool g_bUseGlobalTime	 = true;

uint8_t g_uRows = 5;
uint8_t g_uPage = 1;



#define RANDX random()%SCREEN_WIDTH
#define RANDY random()%SCREEN_HEIGHT
#define RANDC (int)(random()%256)

void SetupMetrics(void)
{
	g_profiler.SetupGraphElement(Profiler::dmCur, g_bLabelsEnabled, g_bGraphEnabled, RGBA(0,255,0));
	g_profiler.SetupGraphElement(Profiler::dmAvg, g_bLabelsEnabled, g_bGraphEnabled, RGBA(0,255,255));
	g_profiler.SetupGraphElement(Profiler::dmMax, g_bLabelsEnabled, g_bGraphEnabled, RGBA(255,0,0));
	g_profiler.SetupGraphElement(Profiler::dmMin, g_bLabelsEnabled, g_bGraphEnabled, RGBA(255,255,0));
}

void init()
{
	set_screen_mode(ScreenMode::hires);

	// set up profiler
	g_profiler.SetDisplaySize(SCREEN_WIDTH, SCREEN_HEIGHT);
	g_profiler.SetRows(g_uRows);
	g_profiler.SetAlpha(200);
	g_profiler.DisplayHistory(g_bDisplayHistory);

	// create probes
	g_pRenderProbe 	= g_profiler.AddProbe("Render", 300); 	// 300 frames of history
	g_pClearProbe 	= g_profiler.AddProbe("Clear", 50, 6); 	// Example of lower resolution, 50 samples with a span of 6 = 300 frames
	g_pRectProbe 		= g_profiler.AddProbe("Rectangle", 300);
	g_pCircleProbe 	= g_profiler.AddProbe("Circle", 300);
	g_pUpdateProbe 	= g_profiler.AddProbe("Update");

	// enable metrics
	SetupMetrics();
}


void render(uint32_t time)
{
	static Point    ptMiddle(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
	static uint32_t lastButtons = 0;

	//static ProfilerProbe *pAwayRenderProbe = g_profiler.AddProbe("AwayFromRender");


	//pAwayRenderProbe->StoreElapsedUs(true);

	uint32_t changedButtons = buttons ^ lastButtons;

	bool button_a = buttons & changedButtons & Button::A;
	bool button_b = buttons & changedButtons & Button::B;
	bool button_x = buttons & changedButtons & Button::X;
	bool button_y = buttons & changedButtons & Button::Y;
	bool button_up = buttons & changedButtons & Button::DPAD_UP;
	bool button_down = buttons & changedButtons & Button::DPAD_DOWN;
	bool button_left = buttons & changedButtons & Button::DPAD_LEFT;
	bool button_right = buttons & changedButtons & Button::DPAD_RIGHT;
	bool button_home = buttons & changedButtons & Button::HOME;

	if(button_up && (g_uRows>1))
	{
		g_uRows --;
		g_profiler.SetRows(g_uRows);
	}

	if(button_down && (g_uRows<11))
	{
		g_uRows ++;
		g_profiler.SetRows(g_uRows);
	}

	if(button_left && (g_uPage > 1))
	{
		g_uPage--;
	}

	if(button_right && (g_uPage < g_profiler.GetPageCount()))
	{
		g_uPage++;
	}

	if(button_a)
	{
		// Graphs
		g_bGraphEnabled = !g_bGraphEnabled;
		SetupMetrics();
	}


	if(button_b)
	{
		// Graphs
		g_bLabelsEnabled = !g_bLabelsEnabled;
		SetupMetrics();
	}

	if(button_x)
	{
		// Graphs
		g_bDisplayHistory = !g_bDisplayHistory;
		g_profiler.DisplayHistory(g_bDisplayHistory);
	}

	if(button_y)
	{
		// change Graphs from time set globally to time set locally
		g_bUseGlobalTime = !g_bUseGlobalTime;
	}

	if(button_home)
	{
		// Log to CDC current values
		g_profiler.LogProbes();
	}

	lastButtons = buttons;

	g_pRenderProbe->Start();

	// clear screen
	g_pClearProbe->Start();
  screen.pen(RGBA(0, 0, 0, 255));
  screen.clear();
  g_pClearProbe->StoreElapsedUs();

  // draw rect
  g_pRectProbe->Start();
 	screen.pen(RGBA(0,0,255,255));
 	Point ptTl = Point(ptMiddle.x-(g_uSize/2), ptMiddle.y-(g_uSize/2));
 	Point ptBr = Point(ptMiddle.x+(g_uSize/2), ptMiddle.y+(g_uSize/2));
  screen.rectangle(Rect(ptTl, ptBr));
  g_pRectProbe->StoreElapsedUs();

  // draw circle
  g_pCircleProbe->Start();
 	screen.pen(RGBA(255,0,0,128));
  screen.circle(ptMiddle, (g_uSizeMax-g_uSize)/2);
  g_pCircleProbe->StoreElapsedUs();


  g_pRenderProbe->StoreElapsedUs();

  if(g_bUseGlobalTime)
  {
    // set global graph time to maximum render time
  	g_profiler.SetGraphTime(g_pRenderProbe->ElapsedMetrics().uMaxElapsedUs);
  }
  else
  {
    // disable global graph time
  	g_profiler.SetGraphTime(0);

  	// set All probes graph time to their maximum logged value
  	g_pRenderProbe->SetGraphTimeUsToMax();;
  	g_pClearProbe->SetGraphTimeUsToMax();;
  	g_pRectProbe->SetGraphTimeUsToMax();;
  	g_pCircleProbe->SetGraphTimeUsToMax();;
  	g_pUpdateProbe->SetGraphTimeUsToMax();;
  }

  // display the overlay
  g_profiler.DisplayProbeOverlay(g_uPage);

}

void update(uint32_t time)
{
	// Scoped profiler probes automatically call Start() and at end of scope call StoreElapsedUs()
	ScopedProfilerProbe scopedProbe(g_pUpdateProbe);

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
