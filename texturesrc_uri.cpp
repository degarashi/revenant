#include "texturesrc_uri.hpp"
#include "sdl_surface.hpp"
#include "sdl_rw.hpp"
#include "gl_if.hpp"

namespace rev {
	namespace {
		struct CnvResult {
			HSfc			surface;
			GLFormatDesc	desc;
			bool			continuous;
		};
		/*
			\param[in]	sfc		ピクセルデータが入ったサーフェスクラス
			\param[in]	fmt		希望するフォーマット(none可)
			\param[in]	bP2		trueなら2の乗数サイズへ変換
		*/
		CnvResult CnvSurface(const HSfc& sfc, const InCompressedFmt_OP fmt, const bool bP2) {
			CnvResult res;
			// SDLフォーマットから適したOpenGLフォーマットへ変換
			res.surface = sfc;
			if(fmt) {
				res.desc = *GLFormat::QueryInfo(*fmt);
			} else {
				// 希望するフォーマットが無ければSurfaceから決定
				auto info = GLFormat::QuerySDLtoGL(res.surface->getFormat().format);
				if(!info) {
					// INDEXEDなフォーマット等は該当が無いのでRGB24として扱う
					info = GLFormat::QuerySDLtoGL(SDL_PIXELFORMAT_RGB24);
					D_Assert0(info);
				}
				res.desc = *info;
			}
			{
				const auto sdlFmt = res.desc.sdlFormat!=SDL_PIXELFORMAT_UNKNOWN ? res.desc.sdlFormat : res.desc.sdlLossFormat;
				res.surface = res.surface->convert(sdlFmt);
			}
			// テクスチャ用のサイズ調整
			if(!bP2 && res.surface->isContinuous()) {
				// 2乗サイズ合わせではなくpitchが詰めてある場合は変換しなくていい
				res.continuous = true;
			} else {
				const auto size = res.surface->getSize();
				const lubee::PowSize n2size{size.width, size.height};
				if(bP2 && size != n2size) {
					// 2乗サイズ合わせ
					res.surface = res.surface->resize(n2size);
				}
				res.continuous = res.surface->isContinuous();
			}
			return res;
		}
		/*
			Leyer0のみ書き込む
			\param[in]	tflag	GL_TEXTURE_2D or GL_TEXTURE_CUBE_MAP_(POSITIVE|NEGATIVE)_(X|Y|Z)
			\param[in]	bMip	trueならミップマップ生成
		*/
		TextureLoadResult WritePixelLayer0(const CnvResult& cnv, const GLenum tflag, const bool bMip) {
			const auto make = [tflag, &desc=cnv.desc](const auto size, const void* data) {
				GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				GL.glTexImage2D(
					tflag,
					0,
					desc.format,
					size.width, size.height,
					0,
					desc.baseFormat,
					desc.elementType,
					data
				);
			};
			const auto size = cnv.surface->getSize();
			if(cnv.continuous) {
				// 2乗サイズ合わせではなくpitchが詰めてある場合は変換しなくていい
				auto lk = cnv.surface->lock();
				make(size, lk.getBits());
			} else {
				const auto buff = cnv.surface->extractAsContinuous();
				make(size, &buff[0]);
			}
			const std::size_t miplevel = (!bMip) ?
				1 :
				lubee::bit::MSB(std::max(size.width,size.height))+1;
			return {
				.size = size,
				.format = cnv.desc.format,
				.miplevel = miplevel
			};
		}
		//! texのfaceにhRWのピクセルデータを書き込む
		TextureLoadResult LoadPixelsFromRW(const GLenum tflag, const HRW& hRW, const InCompressedFmt_OP format, const bool mip) {
			const HSfc sfc = Surface::Load(hRW);
			return WritePixelLayer0(
				CnvSurface(sfc, format, true),
				tflag,
				mip
			);
		}
	}
	TextureLoadResult LoadPixelsFromBuffer(const GLenum tflag, const GLenum format, const lubee::SizeI& size, const ByteBuff& buff, const bool bP2, const bool bMip) {
		// 簡単の為に一旦SDL_Surfaceに変換
		const auto info = GLFormat::QueryInfo(format);
		const int pixelsize = info->numElem* GLFormat::QuerySize(info->baseFormat);
		const HSfc sfc = Surface::Create(buff, pixelsize*size.width, size.width, size.height, info->sdlFormat);
		return WritePixelLayer0(
			CnvSurface(sfc, spi::none, bP2),
			tflag,
			bMip
		);
	}

	// ------------------------- TextureSrc_URI -------------------------
	TextureSrc_URI::TextureSrc_URI(const HURI& uri, const bool mip, const InCompressedFmt_OP fmt):
		TextureSource(fmt, lubee::SizeI(0,0)),
		_uri(uri),
		_miplevel(0),
		_mip(mip)
	{}
	void TextureSrc_URI::onDeviceReset() {
		if(_onDeviceReset()) {
			imm_bind(0);
			const auto res = LoadPixelsFromRW(
								getFaceFlag(),
								mgr_rw.fromURI(*_uri, Access::Read),
								getFormat(),
								_mip
							);
			if(_mip)
				GL.glGenerateMipmap(getTextureFlag());
			_size = res.size;
			_format = res.format;
			_miplevel = res.miplevel;
		}
	}
	bool TextureSrc_URI::isCubemap() const {
		return false;
	}
	std::size_t TextureSrc_URI::getMipLevels() const {
		return _miplevel;
	}
	// ------------------------- TextureSrc_CubeURI -------------------------
	TextureSrc_CubeURI::TextureSrc_CubeURI(
		const HURI& uri0, const HURI& uri1, const HURI& uri2,
		const HURI& uri3, const HURI& uri4, const HURI& uri5,
		bool mip, const InCompressedFmt_OP fmt
	):
		TextureSource(fmt, lubee::SizeI(0,0)),
		_uri{uri0, uri1, uri2, uri3, uri4, uri5},
		_miplevel(0),
		_mip(mip)
	{}
	void TextureSrc_CubeURI::onDeviceReset() {
		if(_onDeviceReset()) {
			imm_bind(0);
			for(int i=0 ; i<6 ; i++) {
				const auto res = LoadPixelsFromRW(
					getFaceFlag(static_cast<CubeFace::e>(i)),
					mgr_rw.fromURI(*_uri[i], Access::Read),
					(i==0) ? spi::none : _format,
					_mip
				);
				if(i==0) {
					_size = res.size;
					_format = res.format;
					_miplevel = res.miplevel;
				}
			}
			if(_mip)
				GL.glGenerateMipmap(getTextureFlag());
		}
	}
	bool TextureSrc_CubeURI::isCubemap() const {
		return true;
	}
	std::size_t TextureSrc_CubeURI::getMipLevels() const {
		return _miplevel;
	}
}
