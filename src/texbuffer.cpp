#include "texbuffer.hpp"
#include "gl/if.hpp"

namespace rev {
	// ------------------- TexBuffer -------------------
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

	// ------------------- MipBuffer -------------------
	MipBuffer::MipBuffer(TexBuffer&& tb):
		TexBuffer(std::move(tb))
	{}
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
	std::pair<std::size_t, lubee::SizeI> MipBuffer::GetLayerOffset(MipLevel level, lubee::SizeI size) {
		std::size_t ofs = 0;
		for(;;) {
			if(level == 0)
				break;
			--level;
			ofs += size.width * size.height;
			if(!NextMipLevel(size))
				break;
		}
		return {
			ofs,
			size
		};
	}
	std::pair<void*, lubee::SizeI> MipBuffer::getLayer(const MipLevel level) {
		const auto [ofs, ls] = GetLayerOffset(level, size);
		const auto unit = *GLFormat::QueryByteSize(baseFormat, elemType);
		return {
			pixels.data() + ofs * unit,
			ls
		};
	}
	std::pair<const void*, lubee::SizeI> MipBuffer::getLayer(const MipLevel level) const {
		return const_cast<MipBuffer&>(*this).getLayer(level);
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
	bool NStepMipLevel(lubee::SizeI& size, uint_fast8_t n) {
		while(n != 0) {
			if(!NextMipLevel(size))
				return false;
			--n;
		}
		return true;
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
