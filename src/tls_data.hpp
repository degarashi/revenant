#pragma once
#include "sdl_tls.hpp"
#include "lubee/src/random.hpp"

namespace rev {
	extern TLS<lubee::RandomMT>	tls_mt;
	struct VideoParam {
		bool bAnisotropic;
	};
	extern TLS<VideoParam> tls_videoParam;
}
