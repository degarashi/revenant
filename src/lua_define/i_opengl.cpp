#include "../luaimpl.hpp"
#include "../gl/resource.hpp"
#include "../gl/texture.hpp"
#include "../gl/renderbuffer.hpp"
#include "../gl/framebuffer.hpp"

DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::GLRBuffer, GLRBuffer,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(getBufferId)(getSize)
)
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::GLFBuffer, GLFBuffer,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(attachRBuffer)(attachTexture)(attachTextureFace)(detach)
)

namespace rev {
	void LuaImport::RegisterOpenGLClass(LuaState& lsc) {
		RegisterClass<GLRBuffer>(lsc);
		RegisterClass<GLFBuffer>(lsc);
	}
}
