#include "glx.hpp"
#include "../gl_if.hpp"
#include "../gl_error.hpp"

namespace rev {
	namespace draw {
		// -------------- VStream --------------
		RUser<VStream> VStream::use() {
			return RUser<VStream>(*this);
		}
		void VStream::use_begin() const {
			if(spVDecl) {
				const GLBufferCore* tmp[MaxVStream];
				for(std::size_t i=0 ; i<MaxVStream ; i++)
					tmp[i] = vbuff[i] ? &(*vbuff[i]) : nullptr;
				spVDecl->apply(tmp, vAttrId);
			}
			if(ibuff)
				ibuff->use_begin();
		}
		void VStream::use_end() const {
			if(spVDecl) {
				GL.glBindBuffer(GL_ARRAY_BUFFER, 0);
				GL.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			}
			if(ibuff)
				ibuff->use_end();
		}
		// -------------- UserFunc --------------
		UserFunc::UserFunc(const Func& f):
			_func(f)
		{}
		void UserFunc::exec() {
			_func();
		}
		// -------------- Tag_Draw --------------
		Draw::Draw(VStream&& vs, const GLenum mode, const GLint first, const GLsizei count):
			DrawBase(std::move(vs)),
			_mode(mode),
			_first(first),
			_count(count)
		{}
		void Draw::exec() {
			auto u = use();
			GL.glDrawArrays(_mode, _first, _count);
			D_GLAssert0();
		}
		// -------------- Tag_DrawI --------------
		DrawIndexed::DrawIndexed(VStream&& vs, const GLenum mode, const GLsizei count, const GLenum sizeF, const GLuint offset):
			DrawBase(std::move(vs)),
			_mode(mode),
			_count(count),
			_sizeF(sizeF),
			_offset(offset)
		{}
		void DrawIndexed::exec() {
			auto u = use();
			GL.glDrawElements(_mode, _count, _sizeF, reinterpret_cast<const GLvoid*>(_offset));
			D_GLAssert0();
		}

		// -------------- Uniforms --------------
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
				// -------- Unsigned Integer --------
				[](GLint id, const void* ptr, const std::size_t n) {
					GL.glUniform1uiv(id, n, reinterpret_cast<const GLuint*>(ptr)); },
				[](GLint id, const void* ptr, const std::size_t n) {
					GL.glUniform2uiv(id, n, reinterpret_cast<const GLuint*>(ptr)); },
				[](GLint id, const void* ptr, const std::size_t n) {
					GL.glUniform3uiv(id, n, reinterpret_cast<const GLuint*>(ptr)); },
				[](GLint id, const void* ptr, const std::size_t n) {
					GL.glUniform4uiv(id, n, reinterpret_cast<const GLuint*>(ptr)); },
			};
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
		void Unif_Vec_Exec(const std::size_t idx, const GLint id, const void* ptr, const std::size_t n) {
			c_iglfV[idx](id, ptr, n);
		}
		void Unif_Mat_Exec(const std::size_t idx, const GLint id, const void* ptr, const std::size_t n, const bool bT) {
			c_iglfM[idx](id, ptr, n, bT ? GL_TRUE : GL_FALSE);
		}
	}
}
