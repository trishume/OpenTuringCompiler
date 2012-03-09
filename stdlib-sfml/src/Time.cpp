#include <math.h>
#include <SFML/System.hpp>

#include "openTuringLibDefs.h"

static sf::Clock mainClock;
static sf::Clock lastDelayClock;

extern "C" {
	TInt Turing_StdlibSFML_Time_Elapsed() {
		return floor(mainClock.GetElapsedTime()*1000);
	}
	void Turing_StdlibSFML_Time_Delay(TInt milliseconds) {
#ifdef OS_WINDOWS
   		Sleep(milliseconds);
#else
  		usleep(milliseconds * 1000);
#endif
	}
	//! delay a certain number of ms since the last call to the function
	void Turing_StdlibSFML_Time_DelaySinceLast(TInt delay) {
		TInt left = delay - floor(lastDelayClock.GetElapsedTime()*1000);
		if(left > 0) Turing_StdlibSFML_Time_Delay(left);
		lastDelayClock.Reset();
	}
}