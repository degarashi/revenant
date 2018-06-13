#include "texturesrc_mem.hpp"
#include "gl_error.hpp"
#include "gl_resource.hpp"
#include "gl_if.hpp"

namespace rev {
	TextureSrc_Mem::TextureSrc_Mem(const bool bCube, GLInSizedFmt fmt, const lubee::SizeI& sz, const bool /*bStream*/, const bool bRestore):
		TextureSource(fmt, sz, bCube),
		// _bStream(bStream),
		_bRestore(bRestore)
	{}
	const GLFormatDesc& TextureSrc_Mem::_prepareBuffer() {
		auto& info = *GLFormat::QueryInfo(*getFormat());
		_typeFormat = info.elementType;
		const auto size = getSize();
		_buff = ByteBuff(size.width * size.height * GLFormat::QuerySize(info.elementType) * info.numElem);
		return info;
	}
	void TextureSrc_Mem::onDeviceLost() {
		if(getTextureId() != 0) {
			if(!mgr_gl.isInDtor() && _bRestore) {
				auto& info = _prepareBuffer();
				_buff = readData(info.baseType, info.elementType, 0);
			}
			TextureSource::onDeviceLost();
		}
	}
	void TextureSrc_Mem::onDeviceReset() {
		if(_onDeviceReset()) {
			const auto size = getSize();
			const auto format = getFormat();
			imm_bind(0);
			GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			if(_bRestore && _buff) {
				// バッファの内容から復元
				GLenum baseFormat = GLFormat::QueryInfo(format.get())->baseType;
				GLWarn(glTexImage2D, getFaceFlag(), 0, format.get(), size.width, size.height, 0, baseFormat, _typeFormat.get(), &_buff->operator [](0));
				// DeviceがActiveな時はバッファを空にしておく
				_buff = spi::none;
				_typeFormat = spi::none;
			} else {
				// とりあえず領域だけ確保しておく
				if(isCubemap()) {
					for(int i=0 ; i<=static_cast<int>(CubeFace::NegativeZ) ; i++)
						GLWarn(glTexImage2D, getFaceFlag(static_cast<CubeFace::e>(i)), 0, format.get(), size.width, size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
				} else
					GLWarn(glTexImage2D, getFaceFlag(), 0, format.get(), size.width, size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			}
		}
	}
	// DeviceLostの時にこのメソッドを読んでも無意味
	void TextureSrc_Mem::writeData(AB_Byte buff, const GLTypeFmt srcFmt, const CubeFace face) {
		// バッファ容量がサイズ以上かチェック
		const auto szInput = GLFormat::QuerySize(srcFmt);
		const auto size = getSize();
		Assert0(buff.getLength() >= size.width * size.height * szInput);
		// DeviceLost中でなければすぐにテクスチャを作成するが、そうでなければ内部バッファにコピーするのみ
		if(getTextureId() != 0) {
			// テクスチャに転送
			const auto tfm = getFormat();
			const auto sz = getSize();
			imm_bind(0);
			GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			GL.glTexImage2D(getFaceFlag(face), 0, tfm.get(), sz.width, sz.height,
							0, GLFormat::QueryInfo(*tfm)->baseType, srcFmt.get(), buff.getPtr());
			GLAssert0();
		} else {
			if(_bRestore) {
				// 内部バッファへmove
				_buff = buff.moveTo();
				_typeFormat = srcFmt;
			}
		}
	}
	void TextureSrc_Mem::writeRect(AB_Byte buff, const lubee::RectI& rect, const GLTypeFmt srcFmt, const CubeFace face) {
		const auto size = getSize();
		const auto format = getFormat();
		#ifdef DEBUG
			const size_t bs = GLFormat::QueryByteSize(format.get(), srcFmt);
			const auto sz = buff.getLength();
			D_Assert0(sz >= bs*rect.width()*rect.height());
		#endif
		if(getTextureId() != 0) {
			const auto fmt = getFormat();
			imm_bind(0);
			// GLテクスチャに転送
			const GLenum baseFormat = GLFormat::QueryInfo(fmt.get())->baseType;
			GL.glTexSubImage2D(getFaceFlag(face), 0, rect.x0, rect.y0, rect.width(), rect.height(), baseFormat, srcFmt.get(), buff.getPtr());
		} else {
			// 内部バッファが存在すればそこに書き込んでおく
			if(_buff) {
				// でもフォーマットが違う時は警告だけ出して何もしない
				if(*_typeFormat != srcFmt) {
					Expect(false, u8"テクスチャのフォーマットが違うので部分的に書き込めない");
				} else {
					auto& b = *_buff;
					auto* dst = &b[size.width * rect.y0 + rect.x0];
					auto* src = buff.getPtr();
					// 1画素のバイト数
					const size_t sz = GLFormat::QueryByteSize(format.get(), _typeFormat.get());
					for(int i=0 ; i<rect.height() ; i++) {
						std::memcpy(dst, src, rect.width());
						dst += size.width;
						src += sz;
					}
				}
			}
		}
	}
}
