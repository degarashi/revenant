#include "texturesrc_mem.hpp"
#include "gl_error.hpp"
#include "gl_resource.hpp"
#include "gl_if.hpp"

namespace rev {
	// ---------------- TextureSrc_Mem::Cache ----------------
	TextureSrc_Mem::Cache::Cache(const GLTypeFmt fmt):
		format(fmt)
	{}

	// ---------------- TextureSrc_Mem ----------------
	TextureSrc_Mem::TextureSrc_Mem(const GLInSizedFmt fmt, const lubee::SizeI& sz, const bool mip, const bool bRestore):
		TextureSource(fmt, sz),
		_restore(bRestore),
		_mip(mip)
	{}
	void TextureSrc_Mem::onDeviceLost() {
		if(getTextureId() != 0) {
			if(!mgr_gl.isInDtor() && _restore) {
				_cache = _backupBuffer();
			}
			TextureSource::onDeviceLost();
		}
	}
	void TextureSrc_Mem::onDeviceReset() {
		if(_onDeviceReset()) {
			imm_bind(0);
			_restoreBuffer(_cache);
			// DeviceがActiveな時はバッファを空にしておく
			_cache = spi::none;
		}
	}
	bool TextureSrc_Mem::_restoreFlag() const noexcept {
		return _restore;
	}
	bool TextureSrc_Mem::_mipFlag() const noexcept {
		return _mip;
	}
	std::size_t TextureSrc_Mem::getMipLevels() const {
		if(_mipFlag())
			return CountMipLevel(getSize());
		return 1;
	}

	// ---------------- TextureSrc_Mem2D ----------------
	TextureSrc_Mem2D::Cache TextureSrc_Mem2D::_backupBuffer() const {
		const auto& info = *GLFormat::QueryInfo(*getFormat());
		Cache cache(info.elementType);
		cache.buff = readData(info.baseFormat, info.elementType, 0);
		return cache;
	}
	void TextureSrc_Mem2D::_restoreBuffer(const Cache_Op& c) {
		const auto size = getSize();
		const auto format = *getFormat();
		const auto flag = getFaceFlag();
		const auto mip = _mipFlag();
		if(c) {
			// バッファの内容から復元
			LoadPixelsFromBuffer(flag, format, size, c->buff, false);
		} else {
			// とりあえず領域だけ確保しておく
			GL.glTexImage2D(
				flag,
				0,
				format,
				size.width, size.height,
				0,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				nullptr
			);
		}
		if(mip)
			GL.glGenerateMipmap(flag);
	}
	// DeviceLostの時にこのメソッドを読んでも無意味
	void TextureSrc_Mem2D::writeData(AB_Byte buff, const GLTypeFmt elem) {
		// バッファ容量がサイズ以上かチェック
		const auto szInput = *GLFormat::QuerySize(elem);
		const auto size = getSize();
		Assert0(buff.getLength() >= size.width * size.height * szInput);
		const auto format = *getFormat();
		// DeviceLost中でなければすぐにテクスチャを作成するが、そうでなければ内部バッファにコピーするのみ
		if(getTextureId() != 0) {
			// テクスチャに転送
			const auto sz = getSize();
			const auto flag = getFaceFlag();
			imm_bind(0);
			GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			GL.glTexImage2D(
				flag,
				0,
				format,
				sz.width, sz.height,
				0,
				GLFormat::QueryInfo(format)->baseFormat,
				elem.get(),
				buff.getPtr()
			);
			if(_mipFlag())
				GL.glGenerateMipmap(flag);
		} else {
			if(_restoreFlag()) {
				// 内部バッファへmove
				_cache = Cache(elem);
				_cache->buff = buff.moveTo();
			}
		}
	}
	void TextureSrc_Mem2D::writeRect(AB_Byte buff, const lubee::RectI& rect, const GLTypeFmt elem) {
		const auto size = getSize();
		const auto format = getFormat();
		#ifdef DEBUG
			const size_t bs = *GLFormat::QueryByteSize(format.get(), elem);
			const auto sz = buff.getLength();
			D_Assert0(sz >= bs*rect.width()*rect.height());
		#endif
		if(getTextureId() != 0) {
			const auto fmt = getFormat();
			const auto flag = getFaceFlag();
			imm_bind(0);
			// GLテクスチャに転送
			const GLenum baseFormat = GLFormat::QueryInfo(fmt.get())->baseFormat;
			GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			GL.glTexSubImage2D(
				flag,
				0,
				rect.x0, rect.y0, rect.width(), rect.height(),
				baseFormat,
				elem.get(),
				buff.getPtr()
			);
			// 効率悪いが、とりあえず
			if(_mipFlag())
				GL.glGenerateMipmap(flag);
		} else {
			// 内部バッファが存在すればそこに書き込んでおく
			if(_cache) {
				// でもフォーマットが違う時は警告だけ出して何もしない
				if(_cache->format != elem) {
					Expect(false, u8"テクスチャのフォーマットが違うので部分的に書き込めない");
				} else {
					auto& b = _cache->buff;
					auto* dst = &b[size.width * rect.y0 + rect.x0];
					auto* src = buff.getPtr();
					// 1画素のバイト数
					const size_t sz = *GLFormat::QueryByteSize(format.get(), _cache->format);
					for(int i=0 ; i<rect.height() ; i++) {
						std::memcpy(dst, src, rect.width());
						dst += size.width;
						src += sz;
					}
				}
			}
		}
	}
	bool TextureSrc_Mem2D::isCubemap() const {
		return false;
	}
}
