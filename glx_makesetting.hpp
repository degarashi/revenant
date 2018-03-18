#pragma once
#include "handle/opengl.hpp"

namespace rev {
	namespace parse {
		struct ValueSetting;
		struct BoolSetting;
	}
	HGLState MakeValueSetting(const parse::ValueSetting& s);
	HGLState MakeBoolSetting(const parse::BoolSetting& s);
}
