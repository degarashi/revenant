#pragma once
#include "glx_id.hpp"
#include "primitive.hpp"

namespace rev {
	using Priority = uint32_t;
	struct DrawTag : IDebugGui {
		using TexAr = std::array<HTex, 4>;

		HTech		technique;
		HPrim		primitive;
		TexAr		idTex;
		Priority	priority;
		float		zOffset;

		DrawTag();

		DEF_DEBUGGUI_NAME
		DEF_DEBUGGUI_PROP
	};
}
