#include "viewport.hpp"
#include "../gl_texture.hpp"
#include "../gl_framebuffer.hpp"
#include "../gl_if.hpp"

namespace rev {
	namespace draw {
		// ------------------------- draw::Viewport -------------------------
		Viewport::Viewport(const FBRect& rect):
			_rect(rect)
		{}
		void Viewport::exec() {
			const auto r = _rect.resolve([](){
				return GLFBufferCore::GetCurrentFBSize();
			});
			GL.glViewport(r.x0, r.y0, r.width(), r.height());
		}
	}
}
