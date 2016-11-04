#pragma once
#include <SDL_video.h>
#include <memory>

namespace rev {
	class Window;
	using SPWindow = std::shared_ptr<Window>;
	class GLContext;
	using GLContext_SP = std::shared_ptr<GLContext>;
	class GLContext {
		//! makeCurrentした時に指定したウィンドウ
		SPWindow		_spWindow;
		SDL_GLContext	_ctx;

		GLContext(const SPWindow& w);
		public:
			static GLContext_SP CreateContext(const SPWindow& w, bool bShare=false);
			~GLContext();
			void makeCurrent(const SPWindow& w);
			void makeCurrent();
			void swapWindow();
			static int SetSwapInterval(int n);
	};
}
