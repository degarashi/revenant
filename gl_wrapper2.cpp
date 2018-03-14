#include "gl_if.hpp"
#include "handler.hpp"
#include <SDL_video.h>

namespace std {
	template <>
	struct hash<rev::GLWrap::MF_Pointer> {
		std::size_t operator()(const rev::GLWrap::MF_Pointer& p) const {
			std::size_t ret = 0xdeadbeef;
			for(auto s : p)
				ret ^= hash<rev::GLWrap::MF_Pointer::value_type>()(s);
			return ret;
		}
	};
}
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
	void GLWrap::loadGLFunc() {
		#define DEF_GLCONST(...)

		// 各種API関数の読み込み
		#define DEF_GLMETHOD(ret_type, num, name, args, argnames) \
			GLWrap::name = nullptr; \
			GLWrap::name = (typename GLWrap::t_##name) GLGETPROC(name); \
			if(GLWrap::name == nullptr) GLWrap::name = (typename GLWrap::t_##name)GLGETPROC(BOOST_PP_CAT(name,ARB)); \
			if(GLWrap::name == nullptr) GLWrap::name = (typename GLWrap::t_##name)GLGETPROC(BOOST_PP_CAT(name,EXT)); \
			Expect(GLWrap::name != nullptr, "could not load OpenGL function: %s", #name)
		#include REV_OPENGL_INCLUDE
		#undef DEF_GLMETHOD

		// 関数名の格納
		MF_Pointer mfp;
		#define DEF_GLMETHOD(ret_type, num, name, args, argnames) \
			{ \
				auto ptr = &IGL::name; \
				std::memcpy(mfp.data(), &ptr, MF_Size); \
				s_funcName[mfp] = #name; \
			}
		#include REV_OPENGL_INCLUDE
		#undef DEF_GLMETHOD
		#undef DEF_GLCONST

		// 各種API関数の読み込み
		// その他OS依存なAPI関数
		g_bglfuncInit = true;
	}
	const char* GLWrap::_GetFunctionName(const MF_Pointer& mfp) {
		const auto itr = s_funcName.find(mfp);
		if(itr != s_funcName.end())
			return itr->second;
		return "function name not found.";
	}

	GLWrap::MF_Map GLWrap::s_funcName;
}
