#include "../../gl_buffer.hpp"
#include "../../imgui/imgui.h"
#include "../entry_field.hpp"

namespace rev {
	namespace {
		const char		*lb_backupSize = "BackupSize",
						*lb_bufferType = "BufferType",
						*lb_drawType = "DrawType",
						*lb_stride = "Stride",
						*lb_openglId = "OpenGL_Id";
	}
	bool GLBuffer::guiEditor(bool) {
		if(auto f = debug::EntryField("GLBuffer", 5)) {
			f.show(lb_openglId, _idBuff);
			f.show(lb_bufferType, _buffType);
			f.show(lb_drawType, _drawType);
			f.show(lb_stride, _stride);
			f.show(lb_backupSize, _buffSize);
		}
		return false;
	}
}
