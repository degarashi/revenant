#include "input_sdlvalue.hpp"
#include <SDL_mouse.h>

namespace rev {
	SpinLock<SDLInputShared>	g_sdlInputShared;
	const uint32_t SDLInputShared::c_buttonId[3] = {
		SDL_BUTTON_LEFT,
		SDL_BUTTON_RIGHT,
		SDL_BUTTON_MIDDLE
	};
	void SDLInputShared::reset() {
		for(auto& b : button)
			b = false;
		wheel_dx = wheel_dy = 0;
		key.clear();
	}
}
