#include "if.hpp"
#include "../msg/handler.hpp"
#include "depend.hpp"
#include "framebuffer.hpp"
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
	bool IGL_Draw::setSwapInterval(const int n) {
		return SDL_GL_SetSwapInterval(n) == 0;
	}
	bool IGL_OtherSingle::setSwapInterval(const int n) {
		bool ret;
		GLW.getDrawHandler().postExec([&ret, n](){
			ret = IGL_Draw().setSwapInterval(n);
		});
		return ret;
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
		#define DEF_GLMETHOD(ret_type, num, name, args, argnames) \
			s_funcName[ToMFPointer(&IGL::name)] = #name;
		#include REV_OPENGL_INCLUDE
		#undef DEF_GLMETHOD
		#undef DEF_GLCONST

		// デフォルト値 (glx_macro.hppに記載がある物のみ対応)
		#define DEF_DEFAULT(name, ...)	\
			s_reset[ToMFPointer(&IGL::name)] = [](){ \
				GL.name(__VA_ARGS__); \
			}
		DEF_DEFAULT(glLineWidth, 1.f);
		DEF_DEFAULT(glFrontFace, GL_CCW);
		DEF_DEFAULT(glCullFace, GL_BACK);
		DEF_DEFAULT(GLDEPTHRANGE, 0, 1);
		s_reset[ToMFPointer(&IGL::glViewport)] = [](){
			const auto s = GLFBufferCore::GetCurrentFBSize();
			GL.glViewport(0, 0, s.width, s.height);
		};
		s_reset[ToMFPointer(&IGL::glScissor)] = [](){
			const auto s = GLFBufferCore::GetCurrentFBSize();
			GL.glScissor(0, 0, s.width, s.height);
		};
		DEF_DEFAULT(glSampleCoverage, 1.f, GL_FALSE);
		DEF_DEFAULT(glStencilFunc, GL_ALWAYS, 0, GLuint(~0));
		DEF_DEFAULT(stencilFuncFront, GL_ALWAYS, 0, GLuint(~0));
		DEF_DEFAULT(stencilFuncBack, GL_ALWAYS, 0, GLuint(~0));
		DEF_DEFAULT(glStencilOp, GL_KEEP, GL_KEEP, GL_KEEP);
		DEF_DEFAULT(stencilOpFront, GL_KEEP, GL_KEEP, GL_KEEP);
		DEF_DEFAULT(stencilOpBack, GL_KEEP, GL_KEEP, GL_KEEP);
		DEF_DEFAULT(glDepthFunc, GL_LESS);
		DEF_DEFAULT(glBlendEquation, GL_FUNC_ADD);
		DEF_DEFAULT(glBlendEquationSeparate, GL_FUNC_ADD, GL_FUNC_ADD);
		DEF_DEFAULT(glBlendFunc, GL_ONE, GL_ZERO);
		DEF_DEFAULT(glBlendFuncSeparate, GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
		DEF_DEFAULT(glBlendColor, 0, 0, 0, 0);
		DEF_DEFAULT(glColorMask, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		DEF_DEFAULT(glDepthMask, GL_TRUE);
		DEF_DEFAULT(glStencilMask, GLuint(~0));
		DEF_DEFAULT(stencilMaskFront, GLuint(~0));
		DEF_DEFAULT(stencilMaskBack, GLuint(~0));
		DEF_DEFAULT(polygonMode, GL_FILL);
		DEF_DEFAULT(glPolygonOffset, 0, 0);
		#undef DEF_DEFAULT

		// 各種API関数の読み込み
		// その他OS依存なAPI関数
		g_bglfuncInit = true;
	}
	GLWrap::ResetF GLWrap::ResetValueFunction(const MF_Pointer& mfp) {
		const auto itr = s_reset.find(mfp);
		if(itr != s_reset.end())
			return itr->second;
		return nullptr;
	}
	bool GLWrap::DefaultBool(const GLenum flag) {
		if(
			flag == GL_DITHER
			#ifdef USE_OPENGLES2
				|| flag == GL_MULTISAMPLE
			#endif
		)
			return true;
		else
			return false;
	}
	const char* GLWrap::GetFunctionName(const MF_Pointer& mfp) {
		const auto itr = s_funcName.find(mfp);
		if(itr != s_funcName.end())
			return itr->second;
		return "function name not found.";
	}

	GLWrap::MF_NameMap GLWrap::s_funcName;
	GLWrap::MF_ResetMap GLWrap::s_reset;
}
