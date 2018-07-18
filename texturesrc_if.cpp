#include "texturesrc_if.hpp"

namespace rev {
	std::size_t CountMipPixels(lubee::SizeI size) {
		std::size_t ret = 0;
		do {
			ret += size.width * size.height;
		} while(NextMipLevel(size));
		return ret;
	}
	// ----------------------- ITextureSource -----------------------
	MipBuffer ITextureSource::readAllLayer(const GLInFmt baseFormat, const GLTypeFmt elem, const CubeFace face) const {
		auto size = getSize();
		const auto npix = CountMipPixels(size);
		const auto unit = *GLFormat::QueryByteSize(baseFormat, elem);
		ByteBuff buff(npix * unit);
		auto* dst = buff.data();
		uint_fast8_t level = 0;
		do {
			const auto lbuff = readData(baseFormat, elem, level, face);
			std::memcpy(dst, lbuff.pixels.data(), lbuff.pixels.size());
			dst += size.width*size.height * unit;
			++level;
		} while(NextMipLevel(size));
		D_Assert0(dst == buff.data()+buff.size());

		MipBuffer ret(baseFormat, elem);
		ret.pixels = std::move(buff);
		ret.size = getSize();
		return ret;
	}
	bool ITextureSource::hasMipmap() const {
		return getMipLevels() > 1;
	}
}
