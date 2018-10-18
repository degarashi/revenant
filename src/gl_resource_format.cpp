#include "gl_resource.hpp"
#include "lcv.hpp"
#include "lctable.hpp"
#include "compiler_macro.hpp"

namespace rev {
	// 先頭のGL_を除く(+3)
	#define DEF_GLCONST(name, value) \
		(*tbl)[(#name)+3] = lua_Integer(value);
	#define DEF_GLMETHOD(...)

	#define DEF_GLCONST2(name, value) \
		(*tbl)[#name] = lua_Integer(value)
	OPTIMIZE_OFF
	void GLRes::LuaExport(LuaState& lsc) {
		auto tbl = std::make_shared<LCTable>();

		#include REV_OPENGL_INCLUDE

		lsc.setField(-1, "Format", tbl);
		tbl->clear();

		DEF_GLCONST2(ClampToEdge, GL_CLAMP_TO_EDGE);
		DEF_GLCONST2(ClampToBorder, GL_CLAMP_TO_BORDER);
		DEF_GLCONST2(MirroredRepeat, GL_MIRRORED_REPEAT);
		DEF_GLCONST2(Repeat, GL_REPEAT);
		DEF_GLCONST2(MirrorClampToEdge, GL_MIRROR_CLAMP_TO_EDGE);
		lsc.setField(-1, "WrapState", tbl);
	}
	OPTIMIZE_RESET
}
