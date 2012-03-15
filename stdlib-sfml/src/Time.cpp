#include <math.h>
#include <SFML/System.hpp>

#include "TuringCommon/LibDefs.h"
#include "WindowManager.h"

static sf::Clock mainClock;
static sf::Clock lastDelayClock;

extern "C" {
	TInt Turing_StdlibSFML_Time_Elapsed() {
		return floor(mainClock.GetElapsedTime()*1000);
	}
    // TODO event checking is not timed so delays will be imprecise
	void Turing_StdlibSFML_Time_Delay(TInt milliseconds) {
        unsigned int incr = milliseconds > 100 ? 50 : 2;        
        for (int i = 0; i < milliseconds; i+=incr) {
            WinMan->surface();
            sf::Sleep(incr/1000.0);
        }
	}
	//! delay a certain number of ms since the last call to the function
	void Turing_StdlibSFML_Time_DelaySinceLast(TInt delay) {
		TInt left = delay - floor(lastDelayClock.GetElapsedTime()*1000);
		if(left > 0) Turing_StdlibSFML_Time_Delay(left);
		lastDelayClock.Reset();
	}
}