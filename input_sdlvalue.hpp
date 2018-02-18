#pragma once
#ifdef WIN32
	#include <intrin.h>
	#include <windows.h>
#endif
#include "spinlock.hpp"
#include "input_sdl_const.hpp"
#include <vector>

namespace rev {
	struct SDLInputShared {
		const static uint32_t c_buttonId[N_SDLMouseButton];
		bool button[N_SDLMouseButton];
		int	wheel_dx,
			wheel_dy;
		using KeyV = std::vector<uint16_t>;
		KeyV	key;

		//! 毎フレーム呼ばれるリセット関数
		void reset();
	};
	extern SpinLock<SDLInputShared>	g_sdlInputShared;
}
