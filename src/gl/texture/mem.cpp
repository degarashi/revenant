#include "mem.hpp"
#include "../error.hpp"
#include "../resource.hpp"
#include "../if.hpp"

namespace rev {
	// ---------------- TextureSrc_Mem ----------------
	TextureSrc_Mem::TextureSrc_Mem(const GLInSizedFmt fmt, const lubee::SizeI& sz, const bool mipEnable, const bool bRestore):
		TextureSource(fmt, sz),
		_restore(bRestore),
		_mipEnable(mipEnable)
	{}
	void TextureSrc_Mem::onDeviceLost() {
		if(getTextureId() != 0) {
			if(!mgr_gl.isInDtor() && _restore) {
				_backupBuffer();
			}
			TextureSource::onDeviceLost();
		}
	}
	void TextureSrc_Mem::onDeviceReset() {
		if(_onDeviceReset()) {
			imm_bind(0);
			_restoreBuffer();
		}
	}
	bool TextureSrc_Mem::_restoreFlag() const noexcept {
		return _restore;
	}
	bool TextureSrc_Mem::_mipEnabled() const noexcept {
		return _mipEnable;
	}
	bool TextureSrc_Mem::_manualMip() const noexcept {
		return _mipEnabled() && !_mipAuto;
	}
	bool TextureSrc_Mem::_autoMip() const noexcept {
		return _mipEnabled() && _mipAuto;
	}
	std::size_t TextureSrc_Mem::getMipLevels() const {
		if(_mipEnabled())
			return CountMipLevel(getSize());
		return 1;
	}
	// ---------------- TextureSrc_Mem2D ----------------
	void TextureSrc_Mem2D::_backupBuffer() const {
		const auto& info = *GLFormat::QueryInfo(*getFormat());
		if(_manualMip()) {
			// 全ての層をバックアップ {[0][1][2]...[N]}
			_cache = readAllLayer(info.baseFormat, info.elementType);
		} else {
			// 一番上の層だけバックアップ
			_cache = readData(info.baseFormat, info.elementType, 0);
		}
	}
	void TextureSrc_Mem2D::_restoreBuffer() {
		const auto size = getSize();
		const auto format = *getFormat();
		const auto flag = getFaceFlag();
		if(_cache) {
			// バッファの内容から復元
			if(_manualMip()) {
				// 全ての層をロード
				_cache->exportToBindingTexture(flag, format);
			} else {
				// 上の層だけロード
				_cache->TexBuffer::exportToBindingTexture(flag, format);
			}
			// DeviceがActiveな時はバッファを空にしておく
			_cache = spi::none;
		} else {
			const MipLevel nl = getMipLevels();
			// とりあえず領域だけ確保しておく
			for(MipLevel i=0 ; i<nl ; i++) {
				GL.glTexImage2D(
					flag,
					i,
					format,
					size.width, size.height,
					0,
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					nullptr
				);
			}
		}
		if(_autoMip())
			GL.glGenerateMipmap(flag);
	}
	void TextureSrc_Mem2D::_writeData(AB_Byte buff, const GLTypeFmt elem, const MipLevel level, const bool mipAuto) {
		_mipAuto = mipAuto;
		Assert0(_manualMip() || level==0);
		const auto format = *getFormat();
		// 1画素のバイト数
		const auto unit = *GLFormat::QueryByteSize(format, elem);
		auto lsize = getSize();
		Assert0(NStepMipLevel(lsize, level));
		// バッファ容量が足りているかチェック
		Assert0(buff.getLength() >= lsize.width * lsize.height * unit);

		// DeviceLost中でなければすぐにテクスチャを作成するが、そうでなければ内部バッファにコピーするのみ
		if(getTextureId() != 0) {
			// テクスチャに転送
			const auto flag = getFaceFlag();
			imm_bind(0);
			GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			GL.glTexImage2D(
				flag,
				level,
				format,
				lsize.width, lsize.height,
				0,
				GLFormat::QueryInfo(format)->baseFormat,
				elem,
				buff.getPtr()
			);
			if(_autoMip())
				GL.glGenerateMipmap(flag);
		} else {
			if(_restoreFlag()) {
				if(!_cache) {
					const auto size = getSize();
					_cache = spi::construct(format.get(), elem);
					_cache->size = size;
					if(_manualMip()) {
						// ミップマップ分のバッファを確保
						_cache->pixels.resize(CountMipPixels(size));
					}
				}
				if(_manualMip()) {
					const auto [dst, ls] = _cache->getLayer(level);
					std::memcpy(dst, buff.getPtr(), ls.width * ls.height * unit);
				} else {
					// 内部バッファへmove
					_cache->pixels = buff.moveTo();
				}
			}
		}
	}
	void TextureSrc_Mem2D::writeData(AB_Byte buff, const GLTypeFmt elem, const MipLevel level) {
		_writeData(std::move(buff), elem, level, false);
	}
	void TextureSrc_Mem2D::writeData(AB_Byte buff, const GLTypeFmt elem) {
		_writeData(std::move(buff), elem, 0, true);
	}

	void TextureSrc_Mem2D::_writeRect(AB_Byte buff, const lubee::RectI& rect, const GLTypeFmt elem, const MipLevel level, const bool mipAuto) {
		// ------------- copied from _writeData() -------------
		_mipAuto = mipAuto;
		Assert0(_manualMip() || level==0);
		const auto format = *getFormat();
		// 1画素のバイト数
		const auto unit = *GLFormat::QueryByteSize(format, elem);
		auto lsize = getSize();
		Assert0(NStepMipLevel(lsize, level));
		// --------------------------------------------------
		// バッファ容量が足りているかチェック
		Assert0(buff.getLength() >= rect.width() * rect.height() * unit);

		const GLenum baseFormat = GLFormat::QueryInfo(format)->baseFormat;
		auto* src = buff.getPtr();
		if(getTextureId() != 0) {
			imm_bind(0);
			// GLテクスチャに転送
			const auto flag = getFaceFlag();
			GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			GL.glTexSubImage2D(
				flag,
				level,
				rect.x0, rect.y0, rect.width(), rect.height(),
				baseFormat,
				elem,
				src
			);
			// 効率悪いが、とりあえず
			if(_autoMip())
				GL.glGenerateMipmap(flag);
		} else {
			if(_restoreFlag()) {
				// 内部バッファに書き込んでおく
				if(!_cache) {
					_cache = spi::construct(baseFormat, elem);
				}
				auto& c = *_cache;
				// フォーマットが違う時は警告だけ出して何もしない
				if(c.elemType != elem) {
					Expect(false, u8"テクスチャのフォーマットが違うので部分的に書き込めない");
				} else {
					const auto [dstp, ls] = c.getLayer(level);
					auto* dst = static_cast<uint8_t*>(dstp) + (ls.width * rect.y0 + rect.x0) * unit;
					const std::size_t w = rect.width(),
										h = rect.height();
					for(std::size_t i=0 ; i<h ; i++) {
						std::memcpy(dst, src, w*unit);
						dst += lsize.width * unit;
						src += w * unit;
					}
				}
			}
		}
	}
	void TextureSrc_Mem2D::writeRect(AB_Byte buff, const lubee::RectI& rect, const GLTypeFmt elem) {
		_writeRect(std::move(buff), rect, elem, 0, true);
	}
	void TextureSrc_Mem2D::writeRect(AB_Byte buff, const lubee::RectI& rect, const GLTypeFmt elem, const MipLevel level) {
		_writeRect(std::move(buff), rect, elem, level, false);
	}
	bool TextureSrc_Mem2D::isCubemap() const {
		return false;
	}
}
