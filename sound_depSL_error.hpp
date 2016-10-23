#pragma once
#include "lubee/error_chk.hpp"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#define SLEC_Base(flag, act, code)		EChk_usercode##flag<::lubee::err::act>(::rev::SLError(), SOURCEPOS, code)
#define SLAssert(func, ...)				SLEC_Base(_a, Trap, func(__VA_ARGS__))
#define SLAssert_M(obj, method, ...)	SLEC_Base(_a, Trap, (*obj)->method(obj, __VA_ARGS__))
#define SLAssert_M0(obj, method)		SLEC_Base(_a, Trap, (*obj)->method(obj))
#define SLWarn(func, ...)				SLEC_Base(_a, Warn, func(__VA_ARGS__))
#define SLWarn_M(obj, method, ...)		SLEC_Base(_a, Warn, (*obj)->method(obj, __VA_ARGS__))
#define SLWarn_M0(obj, method)			SLEC_Base(_a, Warn, (*obj)->method(obj))

#define D_SLAssert(func, ...)			SLEC_Base(_d, Trap, func(__VA_ARGS__))
#define D_SLAssert_M(obj, method, ...)	SLEC_Base(_d, Trap, (*obj)->method(obj, __VA_ARGS__))
#define D_SLAssert_M0(obj, method)		SLEC_Base(_d, Trap, (*obj)->method(obj))
#define D_SLWran(func, ...)				SLEC_Base(_d, Warn, func(__VA_ARGS__))
#define D_SLWran_M(obj, method, ...)	SLEC_Base(_d, Warn, (*obj)->method(obj, __VA_ARGS__))
#define D_SLWran_M0(obj, method)		SLEC_Base(_d, Warn, (*obj)->method(obj))

namespace rev {
	struct SLError {
		static const char* errorDesc(SLresult result);
		void reset() const;
		static const char* getAPIName();
	};
}
