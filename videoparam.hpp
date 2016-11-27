#pragma once
#include "sdl_tls.hpp"

namespace rev {
	struct VideoParam {
		bool bAnisotropic;
	};
	extern TLS<VideoParam> tls_videoParam;
}
