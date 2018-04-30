#include "clear.hpp"
#include "../gl_if.hpp"
#include "../profiler.hpp"

namespace rev {
	namespace draw {
		Clear::Clear(const ClearParam& p):
			_param(p)
		{}
		void Clear::exec() {
			RevProfile(Framebuffer_Clear);
			GLenum flag = 0;
			if(_param.color) {
				flag |= GL_COLOR_BUFFER_BIT;
				auto& c = *_param.color;
				GL.glClearColor(c.x, c.y, c.z, c.w);
			}
			if(_param.depth) {
				flag |= GL_DEPTH_BUFFER_BIT;
				#ifndef USE_OPENGLES2
					GL.glClearDepth
				#else
					GL.glClearDepthf
				#endif
					(*_param.depth);
			}
			if(_param.stencil) {
				flag |= GL_STENCIL_BUFFER_BIT;
				GL.glClearStencil(*_param.stencil);
			}
			// WindowsではDepthMaskをTrueにしないとクリアされない為、一旦値を退避
			GLboolean b;
			GL.glGetBooleanv(GL_DEPTH_TEST, &b);
			GL.glDepthMask(GL_TRUE);
			GL.glClear(flag);
			// DepthMask値を戻す
			GL.glDepthMask(b);
		}
	}
}
