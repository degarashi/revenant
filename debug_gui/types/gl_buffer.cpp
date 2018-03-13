#include "../../gl_buffer.hpp"
#include "../../imgui/imgui.h"
#include "../entry_field.hpp"

namespace rev {
	const char* GLVBuffer::getDebugName() const noexcept {
		return "VertexBuffer";
	}
	bool GLBuffer::property(const bool edit) {
		auto f = debug::EntryField("GLBuffer", edit);
		f.show( "OpenGL_Id", _idBuff);
		f.show( "BufferType", _buffType);
		f.show( "DrawType", _drawType);
		f.show( "Stride", _stride);
		f.show( "BackupSize", _buffSize);
		return f.modified();
	}
	const char* GLIBuffer::getDebugName() const noexcept {
		return "IndexBuffer";
	}
}
