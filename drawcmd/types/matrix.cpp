#include "../../gl_if.hpp"

namespace rev::draw {
	namespace {
		using IGLF_M = void(*)(GLint, const void*, std::size_t, GLboolean);
		const IGLF_M c_iglfM[] = {
			[](GLint id, const void* ptr, const std::size_t n, const GLboolean bT) {
				GL.glUniformMatrix2fv(id, n, bT, reinterpret_cast<const GLfloat*>(ptr)); },
			[](GLint id, const void* ptr, const std::size_t n, const GLboolean bT) {
				GL.glUniformMatrix3fv(id, n, bT, reinterpret_cast<const GLfloat*>(ptr)); },
			[](GLint id, const void* ptr, const std::size_t n, const GLboolean bT) {
				GL.glUniformMatrix4fv(id, n, bT, reinterpret_cast<const GLfloat*>(ptr)); }
		};
	}
	void Unif_Mat_Exec(const std::size_t idx, const GLint id, const void* ptr, const std::size_t n, const bool bT) {
		c_iglfM[idx](id, ptr, n, bT ? GL_TRUE : GL_FALSE);
	}
}
