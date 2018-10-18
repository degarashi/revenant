#include "gl_framebuffer_tmp.hpp"
#include "gl_framebuffer_cmd.hpp"

namespace rev {
	GLFBufferTmp::GLFBufferTmp(const GLuint idFb, const lubee::SizeI& s):
		GLFBufferCore(idFb),
		_size(s)
	{}
	void GLFBufferTmp::attachRBuffer(const Att::e att, const GLuint rb) {
		_attachRenderbuffer(att, rb);
	}
	void GLFBufferTmp::attachTexture(const Att::e att, const GLuint id, const MipLevel level) {
		_attachTexture(att, id, level);
	}
	void GLFBufferTmp::attachCubeTexture(const Att::e att, const GLuint id, const GLuint face, const MipLevel level) {
		_attachCubeTexture(att, face, id, level);
	}
	void GLFBufferTmp::dcmd_export(draw::IQueue& q) const {
		DCmd_Fb::MakeCommand_ApplyOnly(q, getBufferId());
	}
}
