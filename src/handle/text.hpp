#pragma once
#include "common.hpp"

namespace rev {
	class FTFace;
	DEF_HANDLE(FTFace, FT)
	namespace detail {
		class TextObj;
	}
	DEF_HANDLE(detail::TextObj, Text)
}
