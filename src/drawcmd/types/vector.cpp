#include "../../gl/if.hpp"

namespace rev::draw {
	namespace {
		using IGLF_V = void (*)(GLint, const void*, std::size_t);
		const IGLF_V c_iglfV[] = {
			// -------- Float --------
			[](GLint id, const void* ptr, const std::size_t n) {
				GL.glUniform1fv(id, n, reinterpret_cast<const GLfloat*>(ptr)); },
			[](GLint id, const void* ptr, const std::size_t n) {
				GL.glUniform2fv(id, n, reinterpret_cast<const GLfloat*>(ptr)); },
			[](GLint id, const void* ptr, const std::size_t n) {
				GL.glUniform3fv(id, n, reinterpret_cast<const GLfloat*>(ptr)); },
			[](GLint id, const void* ptr, const std::size_t n) {
				GL.glUniform4fv(id, n, reinterpret_cast<const GLfloat*>(ptr)); },
			// -------- Integer --------
			[](GLint id, const void* ptr, const std::size_t n) {
				GL.glUniform1iv(id, n, reinterpret_cast<const GLint*>(ptr)); },
			[](GLint id, const void* ptr, const std::size_t n) {
				GL.glUniform2iv(id, n, reinterpret_cast<const GLint*>(ptr)); },
			[](GLint id, const void* ptr, const std::size_t n) {
				GL.glUniform3iv(id, n, reinterpret_cast<const GLint*>(ptr)); },
			[](GLint id, const void* ptr, const std::size_t n) {
				GL.glUniform4iv(id, n, reinterpret_cast<const GLint*>(ptr)); },
			// -------- Unsigned Integer(or Bool) --------
			[](GLint id, const void* ptr, const std::size_t n) {
				GL.glUniform1uiv(id, n, reinterpret_cast<const GLuint*>(ptr)); },
			[](GLint id, const void* ptr, const std::size_t n) {
				GL.glUniform2uiv(id, n, reinterpret_cast<const GLuint*>(ptr)); },
			[](GLint id, const void* ptr, const std::size_t n) {
				GL.glUniform3uiv(id, n, reinterpret_cast<const GLuint*>(ptr)); },
			[](GLint id, const void* ptr, const std::size_t n) {
				GL.glUniform4uiv(id, n, reinterpret_cast<const GLuint*>(ptr)); },
		};
	}
	void Unif_Vec_Exec(const std::size_t idx, const GLint id, const void* ptr, const std::size_t n) {
		c_iglfV[idx](id, ptr, n);
	}
}
