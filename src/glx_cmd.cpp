#include "glx.hpp"
#include "gl/framebuffer.hpp"
#include "gl/if.hpp"
#include "fbrect.hpp"

namespace rev {
	void GLEffect::DCmd_Clear::Command(const void* p) {
		auto& self = *static_cast<const DCmd_Clear*>(p);
		GLenum flag = 0;
		if(self.bColor) {
			flag |= GL_COLOR_BUFFER_BIT;
			auto& c = self.color;
			GL.glClearColor(c.x, c.y, c.z, c.w);
		}
		if(self.bDepth) {
			flag |= GL_DEPTH_BUFFER_BIT;
			#ifndef USE_OPENGLES2
				GL.glClearDepth
			#else
				GL.glClearDepthf
			#endif
				(self.depth);
		}
		if(self.bStencil) {
			flag |= GL_STENCIL_BUFFER_BIT;
			GL.glClearStencil(self.stencil);
		}
		// WindowsではDepthMaskをTrueにしないとクリアされない為、一旦値を退避
		GLboolean b;
		GL.glGetBooleanv(GL_DEPTH_TEST, &b);
		GL.glDepthMask(GL_TRUE);
		GL.glClear(flag);
		// DepthMask値を戻す
		GL.glDepthMask(b);
	}
	void GLEffect::DCmd_Scissor::Command(const void* p) {
		auto& self = *static_cast<const DCmd_Scissor*>(p);
		const auto r = self.rect.resolve([](){
			return GLFBufferCore::GetCurrentFBSize();
		});
		GL.glScissor(r.x0, r.y0, r.width(), r.height());
	}
	void GLEffect::DCmd_Viewport::Command(const void* p) {
		auto& self = *static_cast<const DCmd_Viewport*>(p);
		const auto r = self.rect.resolve([](){
			return GLFBufferCore::GetCurrentFBSize();
		});
		GL.glViewport(r.x0, r.y0, r.width(), r.height());
	}
}
