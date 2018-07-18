#include "texturesrc_mem.hpp"
#include "gl_error.hpp"
#include "gl_resource.hpp"
#include "gl_if.hpp"

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
		Iter([&c, &info, this](const auto, const auto face){
			const auto seg = readData(info.baseFormat, info.elementType, 0, face);
			c.pixels[face] = std::move(seg.pixels);
		});
	}
	void TextureSrc_MemCube::_restoreBuffer() {
		const auto format = *getFormat();
		const auto size = getSize();
		if(_cache) {
			auto& c = *_cache;
			// バッファの内容から復元
			GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			Iter([&](const auto flag, const auto face){
				GL.glTexImage2D(
					flag,
					0,
					format,
					size.width, size.height,
					0,
					c.baseFormat,
					c.elemType,
					c.pixels[face].data()
				);
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
		if(_mipFlag())
			GL.glGenerateMipmap(getTextureFlag());
	}
	void TextureSrc_MemCube::writeData(AB_Byte buff, const GLTypeFmt elem, const CubeFace face) {
		// バッファ容量がサイズ以上かチェック
		const auto format = *getFormat();
		const auto unit = *GLFormat::QueryByteSize(format, elem);
		const auto size = getSize();
		const auto layersize = size.width * size.height * unit;
		Assert0(buff.getLength() >= layersize);
		// DeviceLost中でなければすぐにテクスチャを作成するが、そうでなければ内部バッファにコピーするのみ
		const auto desc = *GLFormat::QueryInfo(format);
		if(getTextureId() != 0) {
			// テクスチャに転送
			const auto size = getSize();
			imm_bind(0);
			GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			GL.glTexImage2D(
				getFaceFlag(face),
				0,
				format,
				size.width, size.height,
				0,
				desc.baseFormat,
				elem.get(),
				buff.getPtr()
			);
		} else {
			if(_restoreFlag()) {
				auto& c = prepareCache(elem);
				// 内部バッファへcopy
				std::memcpy(
					c.pixels[face].data(),
					buff.getPtr(),
					layersize
				);
			}
		}
	}
	void TextureSrc_MemCube::writeRect(AB_Byte buff, const lubee::RectI& rect, const GLTypeFmt elem, const CubeFace face) {
		const auto size = getSize();
		const auto format = *getFormat();
		// 1画素のバイト数
		const auto unit = *GLFormat::QueryByteSize(format, elem);
		D_Assert0(buff.getLength() >= rect.width()*rect.height() * unit);
		const auto desc = *GLFormat::QueryInfo(format);
		if(getTextureId() != 0) {
			imm_bind(0);
			// GLテクスチャに転送
			GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			GL.glTexSubImage2D(
				getFaceFlag(),
				0,
				rect.x0, rect.y0, rect.width(), rect.height(),
				desc.baseFormat,
				elem.get(),
				buff.getPtr()
			);
		} else {
			// 内部バッファが存在すればそこに書き込んでおく
			if(_cache) {
				// でもフォーマットが違う時は警告だけ出して何もしない
				if(_cache->elemType != elem) {
					Expect(false, u8"テクスチャのフォーマットが違うので部分的に書き込めない");
				} else {
					auto* dst = _cache->pixels[face].data() + (size.width * rect.y0 + rect.x0);
					auto* src = buff.getPtr();
					for(int i=0 ; i<rect.height() ; i++) {
						std::memcpy(dst, src, rect.width());
						dst += size.width;
						src += rect.width() * unit;
					}
				}
			}
		}
	}
	bool TextureSrc_MemCube::isCubemap() const {
		return true;
	}
}
