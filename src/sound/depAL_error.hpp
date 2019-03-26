#pragma once
#include "lubee/src/error_chk.hpp"
#include <AL/al.h>
#include <AL/alc.h>

#define ALEC_Base(flag, act, ...)	EChk_polling##flag<::lubee::err::act>(::rev::ALError(), SOURCEPOS, __VA_ARGS__)
#define ALEC_Base0(flag, act)		EChk_polling##flag<::lubee::err::act>(::rev::ALError(), SOURCEPOS)
#define ALCEC_Base(flag, act, ...)	EChk_polling##flag<::lubee::err::act>(::rev::ALCError(::rev::SoundMgr_depAL::ref().getDevice()), SOURCEPOS, __VA_ARGS__)

#define ALAssert(...)					ALEC_Base(_a, Trap, __VA_ARGS__)
#define ALAssert0()						ALEC_Base0(_a, Trap)
#define ALWarn(...)						ALEC_Base(_a, Warn, __VA_ARGS__)
#define ALWarn0()						ALEC_Base0(_a, Warn)
#define ALCAssert(...)					ALCEC_Base(_a, Trap, __VA_ARGS__)
#define ALCWarn(...)					ALCEC_Base(_a, Warn, __VA_ARGS__)

#define D_ALAssert(...)					ALEC_Base(_d, Trap, __VA_ARGS__)
#define D_ALWarn(...)					ALEC_Base(_d, Warn, __VA_ARGS__)
#define D_ALCAssert(...)				ALCEC_Base(_d, Trap, __VA_ARGS__)
#define D_ALCWarn(...)					ALCEC_Base(_d, Warn, __VA_ARGS__)

namespace rev {
	struct ALError {
		const static std::pair<ALenum, const char*> ErrorList[];
		const char* errorDesc() const;
		void reset() const;
		const char* getAPIName() const;
	};
	struct ALCError {
		ALCdevice* _dev;
		ALCError(ALCdevice* dev);
		const static std::pair<ALCenum, const char*> ErrorList[];
		const char* errorDesc() const;
		void reset() const;
		const char* getAPIName() const;
	};
}
