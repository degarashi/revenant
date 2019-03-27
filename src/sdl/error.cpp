#include "error.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

namespace rev {
	namespace detail {
		// -------------------- SDLErrorI --------------------
		const char* SDLErrorI::Get() {
			return SDL_GetError();
		}
		void SDLErrorI::Reset() {
			SDL_ClearError();
		}
		const char *const SDLErrorI::c_apiName = "SDL2";
		// -------------------- IMGErrorI --------------------
		const char* IMGErrorI::Get() {
			return IMG_GetError();
		}
		void IMGErrorI::Reset() {
			IMG_SetError("");
		}
		const char *const IMGErrorI::c_apiName = "SDL2_image";
	}
}
