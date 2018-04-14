#include "gl_buffer.hpp"
#include "gl_if.hpp"
#include "handler.hpp"
#include "gl_error.hpp"
#include <cstring>
#include "drawtoken/buffer.hpp"

namespace rev {
	using ByteBuff = std::vector<uint8_t>;
	// --------------------------- GLBufferCore ---------------------------
	GLBufferCore::GLBufferCore(const BufferType typ, const DrawType dtype):
		_buffType(typ),
		_drawType(dtype),
		_stride(0),
		_idBuff(0)
	{}
	GLuint GLBufferCore::getBuffId() const {
		return _idBuff;
	}
	BufferType GLBufferCore::getBuffType() const {
		return _buffType;
	}
	GLuint GLBufferCore::getStride() const {
		return _stride;
	}
	void GLBufferCore::use_begin() const {
		GL.glBindBuffer(getBuffType(), getBuffId());
	}
	void GLBufferCore::use_end() const {
		GL.glBindBuffer(getBuffType(), 0);
	}

	// --------------------------- GLBuffer ---------------------------
	void GLBuffer::onDeviceLost() {
		if(_idBuff != 0) {
			GLW.getDrawHandler().postExecNoWait([buffId=getBuffId(), buffType=getBuffType()](){
				GLuint num;
				D_GLWarn(glGetIntegerv, GL_ARRAY_BUFFER_BINDING, reinterpret_cast<GLint*>(&num));
				if(num == buffId)
					D_GLWarn(glBindBuffer, buffType, 0);
				else {
					D_GLWarn(glGetIntegerv, GL_ELEMENT_ARRAY_BUFFER_BINDING, reinterpret_cast<GLint*>(&num));
					if(num == buffId)
						D_GLWarn(glBindBuffer, buffType, 0);
				}
				D_GLWarn(glDeleteBuffers, 1, &buffId);
			});
			_idBuff = 0;
		}
	}
	void GLBuffer::onDeviceReset() {
		if(_idBuff == 0) {
			D_GLWarn(glGenBuffers, 1, &_idBuff);
			if(_buff) {
				const RUser _(*this);
				D_GLWarn(glBufferData, _buffType, _buffSize, _pBuffer, _drawType);
			}
		}
	}
	GLBuffer::GLBuffer(const BufferType typ, const DrawType dtype):
		GLBufferCore(typ, dtype),
		_buffSize(0)
	{}
	GLBuffer::~GLBuffer() {
		onDeviceLost();
	}
	void GLBuffer::_initData() {
		if(getBuffId() > 0) {
			const RUser _(*this);
			D_GLWarn(glBufferData, _buffType, _buffSize, _pBuffer, _drawType);
		}
	}
	void GLBuffer::initData(const void* src, const std::size_t nElem, const GLuint stride) {
		std::size_t sz = nElem*stride;
		ByteBuff b(sz);
		std::memcpy(b.data(), src, sz);
		initData(std::move(b), stride);
	}
	void GLBuffer::updateData(const void* src, const std::size_t nElem, const GLuint offset) {
		std::size_t szCopy = nElem * _stride,
					ofs = offset*_stride;
		std::memcpy(reinterpret_cast<char*>(_pBuffer)+ofs, src, szCopy);
		if(getBuffId() > 0) {
			const RUser _(*this);
			D_GLWarn(glBufferSubData, _buffType, ofs, szCopy, src);
		}
	}
	GLuint GLBuffer::getSize() const {
		return _buffSize;
	}
	GLuint GLBuffer::getNElem() const {
		return _buffSize / _stride;
	}
	draw::Buffer GLBuffer::getDrawToken() const {
		Assert0(getBuffId() > 0);
		return draw::Buffer(*this, const_cast<GLBuffer*>(this)->shared_from_this());
	}
	void* GLBuffer::_getBufferPtr() const noexcept {
		return _pBuffer;
	}
	std::size_t GLBuffer::_getBufferSize() const noexcept {
		return _buffSize;
	}

	// --------------------------- GLVBuffer ---------------------------
	GLVBuffer::GLVBuffer(const DrawType dtype):
		GLBuffer(BufferType::Vertex, dtype)
	{}

	// --------------------------- GLIBuffer ---------------------------
	GLIBuffer::GLIBuffer(const DrawType dtype):
		GLBuffer(BufferType::Index, dtype)
	{}
	void GLIBuffer::initData(const GLubyte* src, const std::size_t nElem) {
		GLBuffer::initData(src, nElem, sizeof(GLubyte));
	}
	void GLIBuffer::initData(const GLushort* src, const std::size_t nElem) {
		GLBuffer::initData(src, nElem, sizeof(GLushort));
	}
	void GLIBuffer::updateData(const GLubyte* src, const std::size_t nElem, const GLuint offset) {
		GLBuffer::updateData(src, nElem, offset*sizeof(GLubyte));
	}
	void GLIBuffer::updateData(const GLushort* src, const std::size_t nElem, const GLuint offset) {
		GLBuffer::updateData(src, nElem, offset*sizeof(GLushort));
	}
	GLenum GLIBuffer::getSizeFlag() const {
		return GetSizeFlag(getStride());
	}
	GLenum GLIBuffer::GetSizeFlag(const int stride) {
		switch(stride) {
			case sizeof(GLubyte):
				return GL_UNSIGNED_BYTE;
			case sizeof(GLushort):
				return GL_UNSIGNED_SHORT;
			case sizeof(GLuint):
				return GL_UNSIGNED_INT;
			default:
				Assert(false, "unknown ibuffer size type");
		}
		return 0;
	}
}
