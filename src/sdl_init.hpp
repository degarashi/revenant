#pragma once
#include <cstdint>

namespace rev {
	//! RAII形式でSDLの初期化 (MInitializerへ食わせる用)
	struct SDLInitializer {
		SDLInitializer(uint32_t flag);
		~SDLInitializer();
	};
	//! RAII形式でSDL_imageの初期化 (MInitializerへ食わせる用)
	struct IMGInitializer {
		IMGInitializer(uint32_t flag);
		~IMGInitializer();
	};
}
