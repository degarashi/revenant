#include "mem.hpp"
#include "../error.hpp"
#include "../resource.hpp"
#include "../if.hpp"

namespace rev {
	// ---------------------- Cache ----------------------
	TextureSrc_MemCube::Cache::Cache(
		const GLInFmt baseFormat,
		const GLTypeFmt elemType,
		const lubee::SizeI size
	):
		baseFormat(baseFormat),
		elemType(elemType)
	{
		const auto unit = *GLFormat::QueryByteSize(baseFormat, elemType);
		for(std::size_t i=0 ; i<6 ; i++) {
			pixels[i].resize(size.width * size.height * unit);
		}
	}

	// ---------------------- TextureSrc_MemCube ----------------------
	TextureSrc_MemCube::Cache& TextureSrc_MemCube::prepareCache(const GLTypeFmt elem) const {
		if(!_cache) {
			const auto format = *getFormat();
			const auto desc = *GLFormat::QueryInfo(format);
			const auto size = getSize();
			_cache = spi::construct(desc.baseFormat, elem, size);
		} else {
			Assert0(_cache->elemType == elem);
		}
		return *_cache;
	}
	template <class CB>
	void TextureSrc_MemCube::Iter(CB&& cb) const {
		for(std::size_t i=0 ; i<=static_cast<std::size_t>(CubeFace::NegativeZ) ; i++) {
			const auto face = static_cast<CubeFace::e>(i);
			cb(getFaceFlag(face), face);
		}
	}
	void TextureSrc_MemCube::_backupBuffer() const {
		const auto& info = *GLFormat::QueryInfo(*getFormat());
		auto& c = prepareCache(info.elementType);
		if(_manualMip()) {
			// 全てのレイヤーをバックアップ
			Iter([&c, &info, this](const auto, const auto face){
				c.pixels[face] = readAllLayer(info.baseFormat, info.elementType, face).pixels;
			});
		} else {
			// 0番レイヤーだけバックアップ
			Iter([&c, &info, this](const auto, const auto face){
				c.pixels[face] = readData(info.baseFormat, info.elementType, 0, face).pixels;
			});
		}
	}
	void TextureSrc_MemCube::_restoreBuffer() {
		const auto format = *getFormat();
		const auto size = getSize();
		if(_cache) {
			auto& c = *_cache;
			// バッファの内容から復元
			GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			Iter([size, format, &c](const auto flag, const auto face){
				MipBuffer mb(c.baseFormat, c.elemType);
				mb.pixels = std::move(c.pixels[face]);
				mb.size = size;
				mb.exportToBindingTexture(flag, format);
			});
			_cache = spi::none;
		} else {
			// とりあえず領域だけ確保しておく
			Iter([&](const auto flag, const auto){
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
			});
		}
		if(_autoMip())
			GL.glGenerateMipmap(getTextureFlag());
	}
	void TextureSrc_MemCube::_writeData(AB_Byte buff, const GLTypeFmt elem, const CubeFace face, const MipLevel level, const bool mipAuto) {
		_mipAuto = mipAuto;
		Assert0(_manualMip() || level==0);
		const auto format = *getFormat();
		// 1画素のバイト数
		const auto unit = *GLFormat::QueryByteSize(format, elem);
		auto size = getSize();
		Assert0(NStepMipLevel(size, level));
		// バッファ容量が足りているかチェック
		Assert0(buff.getLength() >= size.width * size.height * unit);

		// DeviceLost中でなければすぐにテクスチャを作成するが、そうでなければ内部バッファにコピーするのみ
		if(getTextureId() != 0) {
			// テクスチャに転送
			imm_bind(0);
			GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			GL.glTexImage2D(
				getFaceFlag(face),
				level,
				format,
				size.width, size.height,
				0,
				GLFormat::QueryInfo(format)->baseFormat,
				elem.get(),
				buff.getPtr()
			);
		} else {
			if(_restoreFlag()) {
				auto& c = prepareCache(elem);
				if(_manualMip()) {
					MipBuffer mb(c.baseFormat, c.elemType);
					mb.size = getSize();
					const auto [dst, ls] = mb.getLayer(level);
					std::memcpy(dst, buff.getPtr(), ls.width * ls.height * unit);
					c.pixels[face] = std::move(mb.pixels);
				} else {
					Assert0(level == 0);
					// 内部バッファへmove
					c.pixels[face] = buff.moveTo();
				}
			}
		}
	}
	void TextureSrc_MemCube::writeData(AB_Byte buff, const GLTypeFmt elem, const CubeFace face) {
		_writeData(std::move(buff), elem, face, 0, true);
	}
	void TextureSrc_MemCube::writeData(AB_Byte buff, const GLTypeFmt elem, const CubeFace face, const MipLevel level) {
		_writeData(std::move(buff), elem, face, level, false);
	}

	void TextureSrc_MemCube::_writeRect(AB_Byte buff, const lubee::RectI& rect, const GLTypeFmt elem, const CubeFace face, const MipLevel level, const bool mipAuto) {
		// ------------ copied from _writeData() -----------
		_mipAuto = mipAuto;
		Assert0(_manualMip() || level==0);
		const auto format = *getFormat();
		// 1画素のバイト数
		const auto unit = *GLFormat::QueryByteSize(format, elem);
		auto size = getSize();
		Assert0(NStepMipLevel(size, level));
		// --------------------------------------------------
		// バッファ容量が足りているかチェック
		Assert0(buff.getLength() >= rect.width() * rect.height() * unit);

		if(getTextureId() != 0) {
			const auto flag = getFaceFlag(face);
			imm_bind(0);
			// GLテクスチャに転送
			GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			GL.glTexSubImage2D(
				flag,
				0,
				rect.x0, rect.y0, rect.width(), rect.height(),
				GLFormat::QueryInfo(format)->baseFormat,
				elem.get(),
				buff.getPtr()
			);
			// 効率悪いが、とりあえず
			if(_autoMip())
				GL.glGenerateMipmap(flag);
		} else {
			if(_restoreFlag()) {
				// 内部バッファに書き込んでおく
				auto& c = prepareCache(elem);
				if(_manualMip()) {
					const auto [ofs, ls] = MipBuffer::GetLayerOffset(level, getSize());
					auto* dst = c.pixels[face].data() + ofs * unit;
					auto* src = buff.getPtr();
					const int w = rect.width(),
								h = rect.height();
					for(int i=0 ; i<h ; i++) {
						std::memcpy(dst, src, w * unit);
						dst += ls.width * unit;
						src += w * unit;
					}
				} else {
					Assert0(level == 0);
					c.pixels[face] = buff.moveTo();
				}
			}
		}
	}
	void TextureSrc_MemCube::writeRect(AB_Byte buff, const lubee::RectI& rect, GLTypeFmt elem, CubeFace face) {
		_writeRect(std::move(buff), rect, elem, face, 0, true);
	}
	void TextureSrc_MemCube::writeRect(AB_Byte buff, const lubee::RectI& rect, GLTypeFmt elem, CubeFace face, MipLevel level) {
		_writeRect(std::move(buff), rect, elem, face, level, false);
	}
	bool TextureSrc_MemCube::isCubemap() const {
		return true;
	}
}
