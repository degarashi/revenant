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

	bool NextMipLevel(lubee::SizeI& size) {
		uint_fast8_t count = 0;

		size.width >>= 1;
		if(size.width == 0) {
			size.width = 1;
			++count;
		}
		size.height >>= 1;
		if(size.height == 0) {
			size.height = 1;
			++count;
		}

		return count < 2;
	}
	std::size_t CountMipLevel(const lubee::SizeI size) {
		return lubee::bit::MSB(std::max(size.width,size.height))+1;
	}
	std::size_t CountMipPixels(lubee::SizeI size) {
		std::size_t ret = 0;
		do {
			ret += size.width * size.height;
		} while(NextMipLevel(size));
		return ret;
	}
}
