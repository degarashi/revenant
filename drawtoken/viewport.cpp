#include "viewport.hpp"
#include "../gl_texture.hpp"
#include "../gl_framebuffer.hpp"
#include "../gl_if.hpp"

namespace rev {
	namespace draw {
		// ------------------------- draw::Viewport -------------------------
		Viewport::Viewport(const bool bPixel, const lubee::RectF& r):
			_bPixel(bPixel),
			_rect(r)
		{}
		void Viewport::exec() {
			lubee::RectF r = _rect;
			if(!_bPixel) {
				lubee::SizeI s = GLFBufferCore::GetCurrentFBSize();
				r.x0 *= s.width;
				r.x1 *= s.width;
				r.y0 *= s.height;
				r.y1 *= s.height;
			}
			GL.glViewport(r.x0, r.y0, r.width(), r.height());
		}
	}
}
