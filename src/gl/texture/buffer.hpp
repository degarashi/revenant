#pragma once
#include "gl/types.hpp"
#include "gl/format.hpp"
#include "lubee/src/size.hpp"

namespace rev {
	// もう次の層が無い場合はfalseを返す
	bool NextMipLevel(lubee::SizeI& size);
	bool NStepMipLevel(lubee::SizeI& size, uint_fast8_t n);
	std::size_t CountMipLevel(const lubee::SizeI size);
	std::size_t CountMipPixels(lubee::SizeI size);

	using ByteBuff = std::vector<uint8_t>;
	struct TexBuffer {
		ByteBuff		pixels;
		lubee::SizeI	size;
		GLInFmt			baseFormat;
		GLTypeFmt		elemType;

		TexBuffer(GLInFmt baseFormat, GLTypeFmt elemType);
		void exportToBindingTexture(GLenum tflag, GLInCompressedFmt format) const;
	};
	struct MipBuffer : TexBuffer {
		MipBuffer(TexBuffer&& tb);
		using TexBuffer::TexBuffer;

		static std::pair<std::size_t, lubee::SizeI> GetLayerOffset(MipLevel level, lubee::SizeI size);
		std::pair<void*, lubee::SizeI> getLayer(MipLevel level);
		std::pair<const void*, lubee::SizeI> getLayer(MipLevel level) const;
		template <class CB>
		void divideMipBuffer(CB&& cb) {
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
		template <class CB>
		void divideMipBuffer(CB&& cb) const {
			const_cast<MipBuffer&>(*this).divideMipBuffer(cb);
		}
		void exportToBindingTexture(GLenum tflag, GLInCompressedFmt format) const;
	};
}
