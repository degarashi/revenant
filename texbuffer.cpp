#include "texbuffer.hpp"
#include "gl_if.hpp"

namespace rev {
	TexBuffer::TexBuffer(const GLInFmt baseFormat, const GLTypeFmt elemType):
		baseFormat(baseFormat),
		elemType(elemType)
	{}
	void TexBuffer::exportToBindingTexture(const GLenum tflag, const GLInCompressedFmt format) const {
		GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		GL.glTexImage2D(
			tflag,
			0,
			format,
			size.width, size.height,
			0,
			baseFormat,
			elemType,
			pixels.data()
		);
	}

	void MipBuffer::divideMipBuffer(const CBDivide& cb) const {
		auto tsize = size;
		const auto unit = *GLFormat::QueryByteSize(baseFormat, elemType);
		auto* ptr = pixels.data();
		MipLevel level = 0;
		do {
			cb(ptr, tsize, level);
			ptr += tsize.width * tsize.height * unit;
			++level;
		} while(NextMipLevel(tsize));
	}
	void MipBuffer::exportToBindingTexture(const GLenum tflag, const GLInCompressedFmt format) const {
		GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		divideMipBuffer([=](const void* ptr, const lubee::SizeI size, const MipLevel level){
			GL.glTexImage2D(
				tflag,
				level,
				format,
				size.width, size.height,
				0,
				baseFormat,
				elemType,
				ptr
			);
		});
	}
}
