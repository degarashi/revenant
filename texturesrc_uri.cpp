#include "texturesrc_uri.hpp"
#include "sdl_surface.hpp"
#include "sdl_rw.hpp"
#include "gl_if.hpp"

namespace rev {
	std::size_t CountMipLevel(const lubee::SizeI size) {
		return lubee::bit::MSB(std::max(size.width,size.height))+1;
	}
	bool NextMipLevel(lubee::SizeI& size) {
		uint_fast8_t count = 0;

		size.width >>= 1;
		if(size.width == 0) {
			size.width = 1;
			++count;
		}
		size.height >>= 1;
		if(size.height == 0) {
			size.height = 1;
			++count;
		}

		return count < 2;
	}
	namespace {
		struct PixelBuffer {
			GLFormatDesc	desc;
			lubee::SizeI	size;
			ByteBuff		buff;
		};
		/*
			\param[in]	sfc		ピクセルデータが入ったサーフェスクラス
			\param[in]	fmt		希望するフォーマット(none可)
			\param[in]	bP2		trueなら2の乗数サイズへ変換
		*/
		PixelBuffer LoadPixels(HSfc sfc, const InCompressedFmt_OP fmt, const bool bP2) {
			PixelBuffer res;
			// SDLフォーマットから適したOpenGLフォーマットへ変換
			if(fmt) {
				res.desc = *GLFormat::QueryInfo(*fmt);
			} else {
				// 希望するフォーマットが無ければSurfaceから決定
				auto info = GLFormat::QuerySDLtoGL(sfc->getFormat().format);
				if(!info) {
					// INDEXEDなフォーマット等は該当が無いのでRGB24として扱う
					info = GLFormat::QuerySDLtoGL(SDL_PIXELFORMAT_RGB24);
					D_Assert0(info);
				}
				res.desc = *info;
			}
			{
				const auto sdlFmt = res.desc.sdlFormat!=SDL_PIXELFORMAT_UNKNOWN ? res.desc.sdlFormat : res.desc.sdlLossFormat;
				sfc = sfc->convert(sdlFmt);
			}

			auto size = sfc->getSize();
			// テクスチャ用のサイズ調整
			bool continuous;
			if(!bP2 && sfc->isContinuous()) {
				// 2乗サイズ合わせではなくpitchが詰めてある場合は変換しなくていい
				continuous = true;
			} else {
				const lubee::PowSize n2size{size.width, size.height};
				if(bP2 && size != n2size) {
					// 2乗サイズ合わせ
					sfc = sfc->resize(n2size);
					size = sfc->getSize();
				}
				continuous = sfc->isContinuous();
			}
			if(continuous) {
				auto lk = sfc->lock();
				auto* bits = static_cast<const uint8_t*>(lk.getBits());
				res.buff.assign(
					bits,
					bits + sfc->getFormat().BytesPerPixel*size.width*size.height
				);
			} else {
				res.buff = sfc->extractAsContinuous();
			}
			res.size = size;
			return res;
		}
		struct TextureLoadResult {
			lubee::SizeI		size;
			GLInCompressedFmt	format;
		};
		//! texのfaceにhRWのピクセルデータを書き込む
		TextureLoadResult LoadPixelsFromRW(const GLenum tflag, const HRW& hRW, const InCompressedFmt_OP format) {
			const auto pb = LoadPixels(Surface::Load(hRW), format, false);
			GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			GL.glTexImage2D(
				tflag,
				0,
				pb.desc.format,
				pb.size.width, pb.size.height,
				0,
				pb.desc.baseFormat,
				pb.desc.elementType,
				pb.buff.data()
			);
			return {
				.size = pb.size,
				.format = pb.desc.format
			};
		}
	}

	// ------------------------- TextureSrc_URI -------------------------
	TextureSrc_URI::TextureSrc_URI(const HURI& uri, const bool mip, const InCompressedFmt_OP fmt):
		TextureSource(fmt, lubee::SizeI(0,0)),
		_uri(uri),
		_mip(mip)
	{}
	void TextureSrc_URI::onDeviceReset() {
		if(_onDeviceReset()) {
			imm_bind(0);
			const auto res = LoadPixelsFromRW(
								getFaceFlag(),
								mgr_rw.fromURI(*_uri, Access::Read),
								getFormat()
							);
			if(_mip)
				GL.glGenerateMipmap(getTextureFlag());
			_setSizeAndFormat(res.size, res.format);
		}
	}
	bool TextureSrc_URI::isCubemap() const {
		return false;
	}
	std::size_t TextureSrc_URI::getMipLevels() const {
		if(_mip)
			return CountMipLevel(getSize());
		return 1;
	}
	// ------------------------- TextureSrc_CubeURI -------------------------
	TextureSrc_CubeURI::TextureSrc_CubeURI(
		const HURI& uri0, const HURI& uri1, const HURI& uri2,
		const HURI& uri3, const HURI& uri4, const HURI& uri5,
		bool mip, const InCompressedFmt_OP fmt
	):
		TextureSource(fmt, lubee::SizeI(0,0)),
		_uri{uri0, uri1, uri2, uri3, uri4, uri5},
		_mip(mip)
	{}
	void TextureSrc_CubeURI::onDeviceReset() {
		if(_onDeviceReset()) {
			imm_bind(0);
			for(int i=0 ; i<6 ; i++) {
				const auto res = LoadPixelsFromRW(
					getFaceFlag(static_cast<CubeFace::e>(i)),
					mgr_rw.fromURI(*_uri[i], Access::Read),
					(i==0) ? spi::none : getFormat()
				);
				if(i==0) {
					_setSizeAndFormat(res.size, res.format);
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
		if(_mip)
			return CountMipLevel(getSize());
		return 1;
	}
}
