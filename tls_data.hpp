#pragma once
#include "sdl_tls.hpp"
#include "lubee/random.hpp"

namespace rev {
	extern TLS<lubee::RandomMT>	tls_mt;
}
