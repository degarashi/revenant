#include "../gl_if.hpp"
#include "handler.hpp"
#include "gl_error.hpp"

namespace rev {
	TLS<IGL*>	tls_GL;
	void IGL_Draw::stencilFuncFront(const int func, const int ref, const int mask) {
		GLWrap::glStencilFuncSeparate(GL_FRONT, func, ref, mask);
	}
	void IGL_Draw::stencilFuncBack(const int func, const int ref, const int mask) {
		GLWrap::glStencilFuncSeparate(GL_BACK, func, ref, mask);
	}
	void IGL_Draw::stencilOpFront(const int sfail, const int dpfail, const int dppass) {
		GLWrap::glStencilOpSeparate(GL_FRONT, sfail, dpfail, dppass);
	}
	void IGL_Draw::stencilOpBack(const int sfail, const int dpfail, const int dppass) {
		GLWrap::glStencilOpSeparate(GL_BACK, sfail, dpfail, dppass);
	}
	void IGL_Draw::stencilMaskFront(const int mask) {
		GLWrap::glStencilMaskSeparate(GL_FRONT, mask);
	}
	void IGL_Draw::stencilMaskBack(const int mask) {
		GLWrap::glStencilMaskSeparate(GL_BACK, mask);
	}
	void IGL_Draw::polygonMode(const int mode) {
		GLWrap::glPolygonMode(GL_FRONT_AND_BACK, mode);
	}

	void IGL_OtherSingle::stencilFuncFront(const int func, const int ref, const int mask) {
		auto p = GLW.putShared();
		GLW.getDrawHandler().postExec([=](){
			IGL_Draw().stencilFuncFront(func, ref, mask);
		});
	}
	void IGL_OtherSingle::stencilFuncBack(const int func, const int ref, const int mask) {
		auto p = GLW.putShared();
		GLW.getDrawHandler().postExec([=](){
			IGL_Draw().stencilFuncBack(func, ref, mask);
		});
	}
	void IGL_OtherSingle::stencilOpFront(const int sfail, const int dpfail, const int dppass) {
		auto p = GLW.putShared();
		GLW.getDrawHandler().postExec([=](){
			IGL_Draw().stencilOpFront(sfail, dpfail, dppass);
		});
	}
	void IGL_OtherSingle::stencilOpBack(const int sfail, const int dpfail, const int dppass) {
		auto p = GLW.putShared();
		GLW.getDrawHandler().postExec([=](){
			IGL_Draw().stencilOpBack(sfail, dpfail, dppass);
		});
	}
	void IGL_OtherSingle::stencilMaskFront(const int mask) {
		auto p = GLW.putShared();
		GLW.getDrawHandler().postExec([=](){
			IGL_Draw().stencilMaskFront(mask);
		});
	}
	void IGL_OtherSingle::stencilMaskBack(const int mask) {
		auto p = GLW.putShared();
		GLW.getDrawHandler().postExec([=](){
			IGL_Draw().stencilMaskBack(mask);
		});
	}
	void IGL_OtherSingle::polygonMode(const int mode) {
		auto p = GLW.putShared();
		GLW.getDrawHandler().postExec([=](){
			IGL_Draw().polygonMode(mode);
		});
	}
	namespace {
		template <class RET>
		struct CallHandler {
			template <class CB>
			RET operator()(Handler& h, CB cb) const {
				RET ret;
				h.postExec([&](){
					ret = cb();
				});
				return ret;
			}
		};
		template <>
		struct CallHandler<void> {
			template <class CB>
			void operator()(Handler& h, CB cb) const {
				h.postExec([&](){
					cb();
				});
			}
		};
	}
	#define GLCall(func, seq)	GLWrap::func(BOOST_PP_SEQ_ENUM(seq));
	#define DEF_SINGLE_METHOD(ret_type, name, args, argnames) \
		ret_type IGL_OtherSingle::name(BOOST_PP_SEQ_ENUM(args)) { \
			auto p = GLW.putShared(); \
			return CallHandler<ret_type>()(GLW.getDrawHandler(), [=](){ \
				return IGL_Draw().name(BOOST_PP_SEQ_ENUM(argnames)); }); }
	#ifdef DEBUG
		/* デバッグ時:	glFUNC = チェック付き
						glFUNC_NC = チェック無し */
		#define DEF_DRAW_GLEC(act, func, seq) \
			if((uintptr_t)GLWrap::func != (uintptr_t)GLWrap::glGetError) \
				return GLEC_Base(_a, \
							act, \
							[&](){return GLWrap::func(BOOST_PP_SEQ_ENUM(seq));} \
						); \
			return GLCall(func, seq)
	#else
		#define DEF_DRAW_GLEC(act, func, seq) \
			return GLCall(func, seq)
	#endif

	// マクロで分岐
	#define DEF_GLCONST(...)
	#define DEF_GLMETHOD(ret_type, num, name, args, argnames) \
		typename GLWrap::t_##name GLWrap::name = nullptr; \
		ret_type IGL_Draw::name##_NC(BOOST_PP_SEQ_ENUM(args)) { \
			return GLCall(name, argnames) } \
 		ret_type IGL_Draw::name(BOOST_PP_SEQ_ENUM(args)) { \
			DEF_DRAW_GLEC(Warn, name, argnames) } \
		DEF_SINGLE_METHOD(ret_type, name, args, argnames) \
		DEF_SINGLE_METHOD(ret_type, BOOST_PP_CAT(name, _NC), args, argnames)

		#include REV_OPENGL_INCLUDE

	#undef DEF_SINGLE_METHOD
	#undef DEF_GLMETHOD
	#undef DEF_GLCONST
	#undef DEF_DRAW_METHOD
	#undef DEF_DRAW_GLEC
	#undef GLCall

	GLWrap::GLWrap(const bool bShareEnabled):
		_bShare(bShareEnabled),
		_drawHandler(nullptr)
	{}

	void GLWrap::initializeMainThread() {
		Assert0(!tls_GL.initialized());
		if(_bShare)
			tls_GL = &_ctxDraw;
		else
			tls_GL = &_ctxSingle;
	}
	void GLWrap::initializeDrawThread(Handler& handler) {
		Assert0(!tls_GL.initialized());
		_drawHandler = &handler;
		tls_GL = &_ctxDraw;
	}
	void GLWrap::terminateDrawThread() {
		Assert0(tls_GL.initialized());
		Assert0(_drawHandler);
		tls_GL.terminate();
		_drawHandler = nullptr;
	}
	Handler& GLWrap::getDrawHandler() {
		D_Assert0(_drawHandler);
		return *_drawHandler;
	}
	GLWrap::Shared& GLWrap::refShared() {
		return _pShared;
	}
	GLWrap::PutCall GLWrap::putShared() {
		// SpinLockPをRWにした際の応急処置
		// if(!tls_shared)
		// 	tls_shared = SharedPutV();
		// auto& sh = *tls_shared;
		// for(auto& s : _pShared)
		// 	sh.emplace_back(s.second.put());
		return PutCall(this);
	}
	void GLWrap::_putReset() {
		// SpinLockPをRWにした際の応急処置
		// (*tls_shared).clear();
	}
}
