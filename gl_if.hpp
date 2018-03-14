#pragma once
#include "sdl_tls.hpp"
#include "spinlock_rw.hpp"
#include "spine/singleton.hpp"
#include "gl_header.hpp"
#include <unordered_map>
#include <memory>
#include <boost/preprocessor.hpp>
#include <cstring>

namespace rev {
	struct IGL {
		#define DEF_GLCONST(...)
		#define DEF_GLMETHOD(ret_type, num, name, args, argnames) \
			virtual ret_type name(BOOST_PP_SEQ_ENUM(args)) = 0; \
			virtual ret_type name##_NC(BOOST_PP_SEQ_ENUM(args)) = 0;

		#include REV_OPENGL_INCLUDE

		#undef DEF_GLMETHOD
		#undef DEF_GLMETHOD2
		#undef DEF_GLCONST
		virtual void setSwapInterval(int n) = 0;
		virtual void stencilFuncFront(int func, int ref, int mask) = 0;
		virtual void stencilFuncBack(int func, int ref, int mask) = 0;
		virtual void stencilOpFront(int sfail, int dpfail, int dppass) = 0;
		virtual void stencilOpBack(int sfail, int dpfail, int dppass) = 0;
		virtual void stencilMaskFront(int mask) = 0;
		virtual void stencilMaskBack(int mask) = 0;
		virtual void polygonMode(int mode) = 0;
		virtual ~IGL() {}
	};
	//! 直でOpenGL関数を呼ぶ
	struct IGL_Draw : IGL {
		#define DEF_GLCONST(...)
		#define DEF_GLMETHOD(ret_type, num, name, args, argnames) \
			virtual ret_type name(BOOST_PP_SEQ_ENUM(args)) override; \
			virtual ret_type name##_NC(BOOST_PP_SEQ_ENUM(args)) override;

		#include REV_OPENGL_INCLUDE

		void setSwapInterval(int n) override;
		void stencilFuncFront(int func, int ref, int mask)  override;
		void stencilFuncBack(int func, int ref, int mask)  override;
		void stencilOpFront(int sfail, int dpfail, int dppass)  override;
		void stencilOpBack(int sfail, int dpfail, int dppass)  override;
		void stencilMaskFront(int mask)  override;
		void stencilMaskBack(int mask)  override;
		void polygonMode(int mode) override;
	};
	//! DrawThreadにOpenGL関数呼び出しを委託
	struct IGL_OtherSingle : IGL {
		#include REV_OPENGL_INCLUDE

		#undef DEF_GLMETHOD
		#undef DEF_GLMETHOD2
		#undef DEF_GLCONST

		void setSwapInterval(int n) override;
		void stencilFuncFront(int func, int ref, int mask)  override;
		void stencilFuncBack(int func, int ref, int mask)  override;
		void stencilOpFront(int sfail, int dpfail, int dppass)  override;
		void stencilOpBack(int sfail, int dpfail, int dppass)  override;
		void stencilMaskFront(int mask)  override;
		void stencilMaskBack(int mask)  override;
		void polygonMode(int mode) override;
	};
	extern TLS<IGL*>	tls_GL;
	#define GL	(*(*::rev::tls_GL))

	#define GLW	(::rev::GLWrap::ref())
	#ifdef WIN32
		#define APICALL __attribute__((stdcall))
	#elif defined(ANDROID)
		#define APICALL	GL_APICALL
	#else
		#define APICALL GLAPI
	#endif
	class Handler;
	//! OpenGL APIラッパー
	class GLWrap : public spi::Singleton<GLWrap> {
		private:
			IGL_Draw			_ctxDraw;
			IGL_OtherSingle		_ctxSingle;
			bool				_bShare;
			Handler*			_drawHandler;

			// ---- Context共有データ ----
			using SharedP = SpinLockRW<void*, 4>;
			using Shared = std::unordered_map<int, SharedP>;
			Shared		_pShared;

			using SharedPutV = int;
			TLS<SharedPutV>		tls_shared;
			struct Put {
				void operator()(GLWrap* g) const {
					g->_putReset();
				}
			};
			using PutCall = std::unique_ptr<GLWrap, Put>;
			void _putReset();

		public:
			constexpr static std::size_t MF_Size = sizeof(&IGL::setSwapInterval);
			using MF_Pointer = std::array<uint8_t, MF_Size>;
			using MF_Map = std::unordered_map<MF_Pointer, const char*>;
		private:
			static MF_Map s_funcName;
			static const char* _GetFunctionName(const MF_Pointer& mfp);
		public:
			//! 関数ポインタから関数名を取得
			static const char* GetFunctionName(const MF_Pointer& ptr);

			#define DEF_GLCONST(...)
			#define DEF_GLMETHOD(ret_type, num, name, args, argnames) \
				using t_##name = ret_type APICALL(*)(BOOST_PP_SEQ_ENUM(args)); \
				static t_##name name;

			#include REV_OPENGL_INCLUDE

			#undef DEF_GLMETHOD
			#undef DEF_GLCONST
		public:
			GLWrap(bool bShareEnabled);
			void loadGLFunc();
			bool isGLFuncLoaded();
			void initializeMainThread();
			void initializeDrawThread(Handler& handler);
			void terminateDrawThread();

			Handler& getDrawHandler();
			Shared& refShared();
			PutCall putShared();
			template <class Ret, class... Args>
			static const char* GetFunctionName(Ret (IGL::*func)(Args...)) {
				MF_Pointer mfp;
				std::memcpy(mfp.data(), &func, MF_Size);
				return _GetFunctionName(mfp);
			}
	};
	#undef APICALL
}
