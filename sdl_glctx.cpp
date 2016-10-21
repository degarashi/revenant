#include "sdl_glctx.hpp"
#include "sdl_window.hpp"
#include "sdl_error.hpp"

namespace rev {
	GLContext::GLContext(const SPWindow& w) {
		_ctx = SDLAssert(SDL_GL_CreateContext, w->getWindow());
	}
	SPGLContext GLContext::CreateContext(const SPWindow& w, bool bShare) {
		// SharedContext設定なのにCurrentContextがセットされていない場合はエラー
		Assert(!bShare || SDL_GL_GetCurrentContext(), "SharedContext flag has set, however there is no active context");
		Window::SetGLAttributes(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, bShare ? 1 : 0);
		return SPGLContext(new GLContext(w));
	}
	GLContext::~GLContext() {
		makeCurrent();
		D_SDLAssert(SDL_GL_DeleteContext, _ctx);
	}
	void GLContext::makeCurrent(const SPWindow& w) {
		D_SDLAssert(SDL_GL_MakeCurrent, w->getWindow(), _ctx);
		_spWindow = w;
	}
	void GLContext::makeCurrent() {
		D_SDLAssert(SDL_GL_MakeCurrent, nullptr, nullptr);
		_spWindow = nullptr;
	}
	void GLContext::swapWindow() {
		if(_spWindow)
			D_SDLWarn(SDL_GL_SwapWindow, _spWindow->getWindow());
	}
	int GLContext::SetSwapInterval(int n) {
		return SDL_GL_SetSwapInterval(n);
	}
}
