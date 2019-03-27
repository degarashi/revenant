#include "../input/sdlvalue.hpp"
#include <SDL_mouse.h>
#include <algorithm>

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
		text.clear();
		// Shift, Ctrl, Alt, Superのフラグは保持
	}
	namespace {
		template <class Itr, class CB>
		void AdjacentRange(Itr itr, const Itr itrE, CB&& cb) {
			while(itr != itrE) {
				Itr itr2 = itr;
				for(;;) {
					++itr2;
					if(itr2 == itrE) {
						cb(itr, itr2);
						return;
					}
					if(*itr != *itr2) {
						cb(itr, itr2);
						itr = itr2;
						break;
					}
				}
			}
		}
	}
	void SDLInputShared::procKey(const KeyCB& cb) {
		std::stable_sort(key.begin(), key.end(),
			[](const auto& p0, const auto& p1){
				return p0.scancode < p1.scancode;
			}
		);
		AdjacentRange(
			key.cbegin(),
			key.cend(),
			[&cb](const auto from, const auto to) {
				auto to2 = to;
				--to2;
				cb(
					std::distance(from, to),
					*to2
				);
			}
		);
	}
}
