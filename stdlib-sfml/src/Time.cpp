#include <math.h>
#include <SFML/System.hpp>

#include "openTuringLibDefs.h"

static sf::Clock mainClock;

extern "C" {
	TInt Turing_StdlibSFML_Time_Elapsed() {
		return floor(mainClock.GetElapsedTime()*1000);
	}
}