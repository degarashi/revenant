#pragma once
#include "gl_types.hpp"
#include "gl_format.hpp"
#include "lubee/size.hpp"

namespace rev {
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
		using TexBuffer::TexBuffer;
		using CBDivide = std::function<void (const void*, lubee::SizeI, MipLevel)>;
		void divideMipBuffer(const CBDivide& cb) const;
		void exportToBindingTexture(GLenum tflag, GLInCompressedFmt format) const;
	};
	// もう次の層が無い場合はfalseを返す
	bool NextMipLevel(lubee::SizeI& size);
}
