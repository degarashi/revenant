#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include "spine/src/enum.hpp"
#include "spine/src/resmgr.hpp"
#include "spine/src/singleton.hpp"
#include "../../handle/sdl.hpp"
#include "../../handle/text.hpp"
#include "lubee/src/size.hpp"
#include <vector>

namespace rev {
	class FTFace {
		public:
			DefineEnumPair(
				RenderMode,
				((Normal)(FT_RENDER_MODE_NORMAL))
				((Mono)(FT_RENDER_MODE_MONO))
				((LCD)(FT_RENDER_MODE_LCD))
			);
		private:
			struct FaceInfo {
				int	baseline,
					height,
					maxWidth;		// フォントの最大横幅
			};
			struct GlyphInfo {
				const uint8_t* data;
				int advanceX;		// 原点を進める幅
				unsigned int
					nlevel,
					width,			// bitmapの横
					height,			// bitmapの縦
					pitch;			// bitmapのピッチ(バイト)
				int horiBearingX,	// originから右にずらしたサイズ
					horiBearingY;	// baseLineから上に出るサイズ
			};

			FT_Face		_face;
			HRW			_hRW;
			FaceInfo	_faceInfo;
			GlyphInfo	_glyphInfo;
			void _updateFaceInfo();

		public:
			FTFace(FT_Face face, const HRW& hRW=HRW());
			FTFace(FTFace&& f);
			~FTFace();
			void setPixelSizes(lubee::SizeI s);
			void setCharSize(lubee::SizeI s, lubee::SizeI dpi);
			void setSizeFromLine(unsigned int lineHeight);
			//! 文字のビットマップを準備
			void prepareGlyph(char32_t code, RenderMode::e mode, bool bBold, bool bItalic);
			const GlyphInfo& getGlyphInfo() const;
			const FaceInfo& getFaceInfo() const;
			const char* getFamilyName() const;
			const char* getStyleName() const;
			size_t getNFace() const;
			int getFaceIndex() const;
	};
	#define mgr_ft (::rev::FTLibrary::ref())
	class FTLibrary :
		public spi::ResMgr<FTFace>,
		public spi::Singleton<FTLibrary>
	{
		private:
			FT_Library	_lib;
		public:
			FTLibrary();
			~FTLibrary();
			//! メモリまたはファイルシステム上のフォントファイルから読み込む
			HFT newFace(const HRW& hRW, int index);
	};
}
