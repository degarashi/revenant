#include "gl_if.hpp"
#include "handler.hpp"
#include <SDL_video.h>

namespace rev {
	namespace {
		bool g_bglfuncInit = false;
	}
	#define GLGETPROC(name) SDL_GL_GetProcAddress(BOOST_PP_STRINGIZE(name))
	bool GLWrap::isGLFuncLoaded() {
		return g_bglfuncInit;
	}
	void IGL_Draw::setSwapInterval(int n) {
		SDL_GL_SetSwapInterval(n);
	}
	void IGL_OtherSingle::setSwapInterval(int n) {
		GLW.getDrawHandler().postExec([=](){
			IGL_Draw().setSwapInterval(n);
		});
	}

	// OpenGL関数ロード
	// FuncNameで読み込めなければFuncNameARBとFuncNameEXTで試す
	#define GLDEFINE(...)
	#define DEF_GLCONST(...)
	#define DEF_GLMETHOD(ret_type, num, name, args, argnames) \
		GLWrap::name = nullptr; \
		GLWrap::name = (typename GLWrap::t_##name) GLGETPROC(name); \
		if(GLWrap::name == nullptr) GLWrap::name = (typename GLWrap::t_##name)GLGETPROC(BOOST_PP_CAT(name,ARB)); \
		if(GLWrap::name == nullptr) GLWrap::name = (typename GLWrap::t_##name)GLGETPROC(BOOST_PP_CAT(name,EXT)); \
		Expect(GLWrap::name != nullptr, "could not load OpenGL function: %s", #name)
		void GLWrap::loadGLFunc() {
			// 各種API関数
			#include REV_OPENGL_INCLUDE
			// その他OS依存なAPI関数
			g_bglfuncInit = true;
		}
	#undef DEF_GLMETHOD
	#undef DEF_GLCONST
	#undef GLDEFINE
}
