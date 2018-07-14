#include "gl_framebuffer_tmp.hpp"

namespace rev {
	GLFBufferTmp::GLFBufferTmp(const GLuint idFb, const lubee::SizeI& s):
		GLFBufferCore(idFb),
		_size(s)
	{}
	void GLFBufferTmp::attachRBuffer(const Att::e att, const GLuint rb) {
		_attachRenderbuffer(att, rb);
	}
	void GLFBufferTmp::attachTexture(const Att::e att, const GLuint id) {
		_attachTexture(att, id);
	}
	void GLFBufferTmp::attachCubeTexture(const Att::e att, const GLuint id, const GLuint face) {
		_attachCubeTexture(att, face, id);
	}
	void GLFBufferTmp::dcmd_export(draw::IQueue& q) const {
		DCmd_Fb::AddTmp(q, getBufferId());
	}
}
