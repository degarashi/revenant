#pragma once
#ifdef WIN32
	#include <intrin.h>
	#include <windows.h>
#endif
#include "spinlock.hpp"

namespace rev {
	struct SDLInputShared {
		int	wheel_dx,
			wheel_dy;

		//! 毎フレーム呼ばれるリセット関数
		void reset();
	};
	extern SpinLock<SDLInputShared>	g_sdlInputShared;
}
