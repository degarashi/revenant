#include "texturesrc_uri.hpp"
#include "sdl_surface.hpp"
#include "sdl_rw.hpp"
#include "gl_if.hpp"

namespace rev {
	namespace {
		/*
			\param[in]	tflag	GL_TEXTURE_2D or GL_TEXTURE_CUBE_MAP_(POSITIVE|NEGATIVE)_(X|Y|Z)
			\param[in]	sfc		ピクセルデータが入ったサーフェスクラス
			\param[in]	fmt		希望するフォーマット(none可)
			\param[in]	bP2		trueなら2の乗数サイズへ変換
			\param[in]	bMip	trueならミップマップ生成
		*/
		Size_Fmt WritePixelData(const GLenum tflag, const HSfc& sfc, const InCompressedFmt_OP fmt, const bool bP2, const bool bMip) {
			// SDLフォーマットから適したOpenGLフォーマットへ変換
			HSfc tsfc = sfc;
			GLFormatDesc desc;
			if(fmt) {
				desc = *GLFormat::QueryInfo(*fmt);
			} else {
				// 希望するフォーマットが無ければSurfaceから決定
				auto info = GLFormat::QuerySDLtoGL(tsfc->getFormat().format);
				if(!info) {
					// INDEXEDなフォーマット等は該当が無いのでRGB24として扱う
					info = GLFormat::QuerySDLtoGL(SDL_PIXELFORMAT_RGB24);
					D_Assert0(info);
				}
				desc = *info;
			}
			const auto sdlFmt = desc.sdlFormat!=SDL_PIXELFORMAT_UNKNOWN ? desc.sdlFormat : desc.sdlLossFormat;
			tsfc = tsfc->convert(sdlFmt);
			// テクスチャ用のサイズ調整
			auto size = tsfc->getSize();
			const lubee::PowSize n2size{size.width, size.height};
			using CB = std::function<void (const void*)>;
			std::function<void (CB)>	func;
			// 2乗サイズ合わせではなくpitchが詰めてある場合は変換しなくていい
			if(!bP2 && tsfc->isContinuous()) {
				func = [&tsfc](CB cb) {
					auto lk = tsfc->lock();
					cb(lk.getBits());
				};
			} else {
				// 2乗サイズ合わせ
				if(bP2 && size != n2size)
					tsfc = tsfc->resize(n2size);
				func = [&tsfc, sdlFmt](CB cb) {
					auto buff = tsfc->extractAsContinuous(sdlFmt);
					cb(&buff[0]);
				};
			}
			// ミップマップの場合はサイズを縮小しながらテクスチャを書き込む
			const auto tsize = tsfc->getSize();
			size = tsize;
			int layer = 0;
			const auto make = [tflag, &layer, &desc, &size](const void* data) {
				GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				GL.glTexImage2D(tflag, layer++, desc.format, size.width, size.height, 0, desc.baseType, desc.elementType, data);
			};
			if(!bMip)
				func(make);
			else {
				for(;;) {
					func(make);
					if(size.width==1 && size.height==1)
						break;
					size.shiftR_one(1);
					tsfc = tsfc->resize(size);
				}
			}
			return std::make_pair(tsize, desc.format);
		}
		//! texのfaceにhRWのピクセルデータを書き込む
		Size_Fmt LoadTexture(const TextureSource& tex, const HRW& hRW, const bool mip, const CubeFace face) {
			const HSfc sfc = Surface::Load(hRW);
			const GLenum tflag = tex.getFaceFlag(face);
			tex.imm_bind(0);
			return WritePixelData(tflag, sfc, tex.getFormat(), true, mip);
		}
	}
	Size_Fmt LoadTextureFromBuffer(const TextureSource& tex, const GLenum tflag, const GLenum format, const lubee::SizeI& size, const ByteBuff& buff, const bool bP2, const bool bMip) {
		// 簡単の為に一旦SDL_Surfaceに変換
		const auto info = GLFormat::QueryInfo(format);
		const int pixelsize = info->numElem* GLFormat::QuerySize(info->baseType);
		const HSfc sfc = Surface::Create(buff, pixelsize*size.width, size.width, size.height, info->sdlFormat);
		tex.imm_bind(0);
		return WritePixelData(tflag, sfc, spi::none, bP2, bMip);
	}

	// ------------------------- TextureSrc_URI -------------------------
	TextureSrc_URI::TextureSrc_URI(const HURI& uri, const bool mip, const InCompressedFmt_OP fmt):
		TextureSource(fmt, lubee::SizeI(0,0), false),
		_uri(uri),
		_mip(mip)
	{}
	void TextureSrc_URI::onDeviceReset() {
		if(_onDeviceReset())
			std::tie(_size, _format) = LoadTexture(
											*this,
											mgr_rw.fromURI(*_uri, Access::Read),
											_mip,
											CubeFace::PositiveX
										);
	}
	// ------------------------- TextureSrc_CubeURI -------------------------
	TextureSrc_CubeURI::TextureSrc_CubeURI(
		const HURI& uri0, const HURI& uri1, const HURI& uri2,
		const HURI& uri3, const HURI& uri4, const HURI& uri5,
		bool mip, const InCompressedFmt_OP fmt
	):
		TextureSource(fmt, lubee::SizeI(0,0), true),
		_uri{uri0, uri1, uri2, uri3, uri4, uri5},
		_mip(mip)
	{}
	void TextureSrc_CubeURI::onDeviceReset() {
		if(_onDeviceReset()) {
			for(int i=0 ; i<6 ; i++) {
				const auto size_fmt = LoadTexture(
					*this,
					mgr_rw.fromURI(*_uri[i], Access::Read),
					_mip,
					static_cast<CubeFace::e>(i)
				);
				if(i==0)
					std::tie(_size, _format) = size_fmt;
			}
		}
	}
}
