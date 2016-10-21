#include "input_sdlvalue.hpp"

namespace rev {
	SpinLock<SDLInputShared>	g_sdlInputShared;
	void SDLInputShared::reset() {
		wheel_dx = wheel_dy = 0;
	}
}
