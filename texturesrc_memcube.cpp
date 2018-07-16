#include "texturesrc_mem.hpp"
#include "gl_error.hpp"
#include "gl_resource.hpp"
#include "gl_if.hpp"

namespace rev {
	template <class CB>
	void TextureSrc_MemCube::Iter(CB&& cb) const {
		for(std::size_t i=0 ; i<=static_cast<std::size_t>(CubeFace::NegativeZ) ; i++) {
			const auto face = static_cast<CubeFace::e>(i);
			cb(getFaceFlag(face), face);
		}
	}
	TextureSrc_MemCube::Cache TextureSrc_MemCube::_backupBuffer() const {
		const auto& info = *GLFormat::QueryInfo(*getFormat());
		Cache cache(info.elementType);
		Iter([&](const auto, const auto face){
			const auto seg = readData(info.baseFormat, info.elementType, face);
			std::copy(seg.begin(), seg.end(), std::back_inserter(cache.buff));
		});
		return cache;
	}
	void TextureSrc_MemCube::_restoreBuffer(const Cache_Op& c) {
		const auto format = *getFormat();
		const auto size = getSize();
		if(c) {
			// バッファの内容から復元
			const GLenum baseFormat = GLFormat::QueryInfo(format)->baseFormat;
			D_Assert0(c->buff.size() % 6 == 0);
			const auto ofs = c->buff.size() / 6;
			GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			Iter([&](const auto flag, const auto face){
				GL.glTexImage2D(
					flag,
					0,
					format,
					size.width, size.height,
					0,
					baseFormat,
					c->format,
					c->buff.data() + ofs*face
				);
			});
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
	void TextureSrc_MemCube::writeData(AB_Byte buff, const GLTypeFmt srcFmt, const CubeFace face) {
		// バッファ容量がサイズ以上かチェック
		const auto szInput = GLFormat::QuerySize(srcFmt);
		const auto size = getSize();
		Assert0(buff.getLength() >= size.width * size.height * szInput);
		// DeviceLost中でなければすぐにテクスチャを作成するが、そうでなければ内部バッファにコピーするのみ
		if(getTextureId() != 0) {
			// テクスチャに転送
			const auto format = *getFormat();
			const auto size = getSize();
			imm_bind(0);
			GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			GL.glTexImage2D(
				getFaceFlag(face),
				0,
				format,
				size.width, size.height,
				0,
				GLFormat::QueryInfo(format)->baseFormat,
				srcFmt.get(),
				buff.getPtr()
			);
		} else {
			if(_restoreFlag()) {
				_cache = Cache(srcFmt);
				_cache->buff.resize(buff.getLength()*6);
				auto& dst = _cache->buff;
				// 内部バッファへcopy
				std::memcpy(
					dst.data() + dst.size()/6 * static_cast<std::size_t>(face),
					buff.getPtr(),
					dst.size()/6
				);
			}
		}
	}
	void TextureSrc_MemCube::writeRect(AB_Byte buff, const lubee::RectI& rect, const GLTypeFmt srcFmt, const CubeFace face) {
		const auto size = getSize();
		const auto format = getFormat();
		#ifdef DEBUG
			const size_t bs = GLFormat::QueryByteSize(format.get(), srcFmt);
			const auto sz = buff.getLength();
			D_Assert0(sz >= bs*rect.width()*rect.height());
		#endif
		if(getTextureId() != 0) {
			const auto format = *getFormat();
			imm_bind(0);
			// GLテクスチャに転送
			const GLenum baseFormat = GLFormat::QueryInfo(format)->baseFormat;
			GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			GL.glTexSubImage2D(
				getFaceFlag(),
				0,
				rect.x0, rect.y0, rect.width(), rect.height(),
				baseFormat,
				srcFmt.get(),
				buff.getPtr()
			);
		} else {
			// 内部バッファが存在すればそこに書き込んでおく
			if(_cache) {
				// でもフォーマットが違う時は警告だけ出して何もしない
				if(_cache->format != srcFmt) {
					Expect(false, u8"テクスチャのフォーマットが違うので部分的に書き込めない");
				} else if(_cache->buff.size() != buff.getLength()*6) {
					Expect(false, u8"テクスチャのサイズが違うので部分的に書き込めない");
				} else {
					auto* dst = _cache->buff.data() + (_cache->buff.size()/6 * static_cast<std::size_t>(face)) + (size.width * rect.y0 + rect.x0);
					auto* src = buff.getPtr();
					// 1画素のバイト数
					const size_t sz = GLFormat::QueryByteSize(format.get(), _cache->format);
					for(int i=0 ; i<rect.height() ; i++) {
						std::memcpy(dst, src, rect.width());
						dst += size.width;
						src += sz;
					}
				}
			}
		}
	}
	bool TextureSrc_MemCube::isCubemap() const {
		return true;
	}
}
