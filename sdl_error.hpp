#pragma once
#include "lubee/error_chk.hpp"

namespace rev {
	namespace detail {
		struct SDLErrorI {
			static const char* Get();
			static void Reset();
			static const char *const c_apiName;
		};
		struct IMGErrorI {
			static const char* Get();
			static void Reset();
			static const char *const c_apiName;
		};
	}
	template <class I>
	struct ErrorT {
		std::string		_errMsg;
		const char* errorDesc() {
			const char* err = I::Get();
			if(*err != '\0') {
				_errMsg = err;
				I::Reset();
				return _errMsg.c_str();
			}
			return nullptr;
		}
		void reset() const {
			I::Reset();
		}
		const char* getAPIName() const {
			return I::c_apiName;
		}
	};
	using SDLError = ErrorT<detail::SDLErrorI>;
	using IMGError = ErrorT<detail::IMGErrorI>;
}

#define SDLEC_Base(flag, act, ...)		EChk_polling##flag<::lubee::err::act>(::rev::SDLError(), SOURCEPOS, __VA_ARGS__)
#define SDLEC_Base0(flag, act)			EChk_polling##flag<::lubee::err::act>(::rev::SDLError(), SOURCEPOS)

#define SDLAssert(...)					SDLEC_Base(_a, Trap, __VA_ARGS__)
#define SDLWarn(...)					SDLEC_Base(_a, Warn, __VA_ARGS__)
#define SDLAssert0()					SDLEC_Base0(_a, Trap)
#define SDLWarn0()						SDLEC_Base0(_a, Warn)
#define D_SDLAssert(...)				SDLEC_Base(_d, Trap, __VA_ARGS__)
#define D_SDLWarn(...)					SDLEC_Base(_d, Warn, __VA_ARGS__)
#define D_SDLAssert0()					SDLEC_Base0(_d, Trap)
#define D_SDLWarn0()					SDLEC_Base0(_d, Warn)
#define SDLThrow(typ, ...)				SDLEC_Base(_a, Throw<typ>, __VA_ARGS__)

#define IMGEC_Base(flag, act, ...)		EChk_polling##flag<::lubee::err::act>(::rev::IMGError(), SOURCEPOS, __VA_ARGS__)
#define IMGEC_Base0(flag, act)			EChk_polling##flag<::lubee::err::act>(::rev::IMGError(), SOURCEPOS)

#define IMGAssert(...)					IMGEC_Base(_a, Trap, __VA_ARGS__)
#define IMGWarn(...)					IMGEC_Base(_a, Warn, __VA_ARGS__)
#define IMGAssert0()					IMGEC_Base0(_a, Trap)
#define IMGWarn0()						IMGEC_Base0(_a, Warn)
#define D_IMGAssert(...)				IMGEC_Base(_d, Trap, __VA_ARGS__)
#define D_IMGWarn(...)					IMGEC_Base(_d, Warn, __VA_ARGS__)
#define D_IMGAssert0()					IMGEC_Base0(_d, Trap)
#define D_IMGWarn0()					IMGEC_Base0(_d, Warn)
#define IMGThrow(typ, ...)				IMGEC_Base(_a, Throw<typ>, __VA_ARGS__)
