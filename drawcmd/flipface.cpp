#include "flipface.hpp"
#include "../gl_if.hpp"

namespace rev::draw {
	void FlipFace::Command(const void*) {
		GLint face;
		GL.glGetIntegerv(GL_FRONT_FACE, &face);
		if(face == GL_CCW)
			face = GL_CW;
		else
			face = GL_CCW;
		GL.glFrontFace(face);
	}
}
