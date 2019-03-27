#pragma once
#include <SDL_video.h>
#include "handle/sdl.hpp"

namespace rev {
	class GLContext;
	using GLContext_SP = std::shared_ptr<GLContext>;
	class GLContext {
		//! makeCurrentした時に指定したウィンドウ
		HWin			_spWindow;
		SDL_GLContext	_ctx;

		GLContext(const HWin& w);
		public:
			static GLContext_SP CreateContext(const HWin& w, bool bShare=false);
			~GLContext();
			void makeCurrent(const HWin& w);
			void makeCurrent();
			void swapWindow();
			static int SetSwapInterval(int n);
	};
}
