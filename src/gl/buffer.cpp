#include "buffer.hpp"
#include "if.hpp"
#include "../msg/handler.hpp"
#include "error.hpp"
#include "../drawcmd/queue_if.hpp"
#include <cstring>

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
				use_begin();
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
			use_begin();
			D_GLWarn(glBufferData, _buffType, _buffSize, _pBuffer, _drawType);
		}
	}
	void GLBuffer::initData(const void* src, const std::size_t size) {
		ByteBuff b(size);
		std::memcpy(b.data(), src, size);
		initData(std::move(b), 0);
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
			use_begin();
			D_GLWarn(glBufferSubData, _buffType, ofs, szCopy, src);
		}
	}
	GLuint GLBuffer::getSize() const {
		return _buffSize;
	}
	GLuint GLBuffer::getNElem() const {
		return _buffSize / _stride;
	}
	void GLBuffer::dcmd_export(draw::IQueue& q) const {
		Assert0(getBuffId() > 0);
		q.add(DCmd_Use{static_cast<const GLBufferCore&>(*this)});
		q.stockResource(shared_from_this());
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
		GLBuffer::initData(src, nElem, sizeof(*src));
	}
	void GLIBuffer::initData(const GLushort* src, const std::size_t nElem) {
		GLBuffer::initData(src, nElem, sizeof(*src));
	}
	void GLIBuffer::initData(const GLuint* src, const std::size_t nElem) {
		GLBuffer::initData(src, nElem, sizeof(*src));
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

	void GLBuffer::DCmd_Use::Command(const void* p) {
		auto& self = *static_cast<const DCmd_Use*>(p);
		self.use_begin();
	}
}
