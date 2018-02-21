#pragma once
#ifdef WIN32
	#include <intrin.h>
	#include <windows.h>
#endif
#include "spinlock.hpp"
#include "input_sdl_const.hpp"
#include <vector>
#include "input_common.hpp"

namespace rev {
	struct SDLInputShared {
		const static uint32_t c_buttonId[N_SDLMouseButton];
		bool button[N_SDLMouseButton];
		int	wheel_dx,
			wheel_dy;
		using KeyV = std::vector<KeyLog>;
		KeyV	key;
		KeyAux	keyaux;
		std::string text;

		// length, KeyLog
		using KeyCB = std::function<void (int, const KeyLog&)>;
		void procKey(const KeyCB& cb);

		//! 毎フレーム呼ばれるリセット関数
		void reset();
	};
	extern SpinLock<SDLInputShared>	g_sdlInputShared;
}
