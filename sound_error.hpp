#pragma once
#include "lubee/error_chk.hpp"

#define OVEC_Base(flag, act, ...)	EChk_return##flag<::lubee::err::act>(::rev::OVError(), SOURCEPOS, __VA_ARGS__)
#define OVAssert(...)				OVEC_Base(_a, Trap, __VA_ARGS__)
#define OVWarn(...)					OVEC_Base(_a, Warn, __VA_ARGS__)
#define D_OVAssert(...)				OVEC_Base(_d, Trap, __VA_ARGS__)
#define D_OVWarn(...)				OVEC_Base(_d, Warn, __VA_ARGS__)

namespace rev {
	struct OVError {
		const static std::pair<int, const char*> ErrorList[];
		const char* errorDesc(int err) const noexcept;
		const char* getAPIName() const noexcept;
	};
}
