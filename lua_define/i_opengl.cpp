#include "../luaimpl.hpp"
#include "../gl_resource.hpp"
#include "../gl_texture.hpp"
#include "../gl_renderbuffer.hpp"
#include "../gl_framebuffer.hpp"

DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::IGLTexture, IGLTexture,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(getResourceName)(getTextureId)(setFilter)(setLinear)(setAnisotropicCoeff)
	(setUVWrap)(setWrap)(getSize)(getTextureId)(getTexFlag)(getFaceFlag)(isMipmap)(save)(isCubemap)
)
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
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::GLRes, GLRes,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(loadTexture)(loadCubeTexture)(createTexture)(createCubeTexture)(makeFBuffer)(makeRBuffer)
)

namespace rev {
	void LuaImport::RegisterOpenGLClass(LuaState& lsc) {
		RegisterClass<IGLTexture>(lsc);
		RegisterClass<GLRBuffer>(lsc);
		RegisterClass<GLFBuffer>(lsc);
		ImportClass(lsc, "System", "glres", &mgr_gl);
	}
}
