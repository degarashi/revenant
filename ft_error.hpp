#pragma once
#include "lubee/error_chk.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#define FTEC_Base(flag, act, ...)		EChk_return##flag<::lubee::err::act>(::rev::FTError(), SOURCEPOS, __VA_ARGS__)
#define FTAssert(...)					FTEC_Base(_a, Trap, __VA_ARGS__)
#define FTAssert0()						FTEC_Base(_a, Trap)
#define D_FTAssert(...)					FTEC_Base(_d, Trap, __VA_ARGS__)
#define D_FTAssert0()					FTEC_Base(_d, Trap)

namespace rev {
	struct FTError {
		const char* errorDesc(int result) const;
		const char* getAPIName() const;
	};
}
