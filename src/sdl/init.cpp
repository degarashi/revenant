#include "init.hpp"
#include "tls.hpp"
#include <SDL_image.h>

namespace rev {
	TLS<SDL_threadID> tls_threadID(~0);
	TLS<std::string> tls_threadName("john doe");
	// -------------------- SDLInitializer --------------------
	SDLInitializer::SDLInitializer(const uint32_t flag) {
		SDL_Init(flag);
	}
	SDLInitializer::~SDLInitializer() {
		SDL_Quit();
	}
	// -------------------- IMGInitializer --------------------
	IMGInitializer::IMGInitializer(const uint32_t flag) {
		IMG_Init(flag);
	}
	IMGInitializer::~IMGInitializer() {
		IMG_Quit();
	}
}

