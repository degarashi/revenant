#include "fontft.hpp"
#include "fontfamily.hpp"
#include "wrap.hpp"
#include "../convert.hpp"
#include "../exception.hpp"
#include "lubee/src/error.hpp"
#include <cstring>

namespace rev {
	namespace {
		// FontIdフラグにしたがってFTFaceにサイズを指定
		void SetFTSize(FTFace& ft, const FontId fontId, const lubee::SizeI dpi) {
			const lubee::SizeI
					size{
						fontId.at<FontId::Width>(),
						fontId.at<FontId::Height>()
					};
			switch(fontId.at<FontId::SizeType>()) {
				case FontId::SizeType_Pixel:
					ft.setPixelSizes(size); break;
				case FontId::SizeType_Point:
					ft.setCharSize(size, dpi); break;
				case FontId::SizeType_LineHeight:
					ft.setSizeFromLine(size.height); break;
				default:
					D_Assert(false, "invalid sizetype number");
			}
		}
		void PrepareGlyph(FTFace& ft, const CharId cid) {
			ft.prepareGlyph(
				cid.code,
				(cid.at<FontId::CharFlag>() & FontId::CharFlag_AA) ? FTFace::RenderMode::Normal : FTFace::RenderMode::Mono,
				cid.at<FontId::Weight>()>0,
				cid.at<FontId::Italic>()
			);
		}
	}
	// TODO: 縁取り対応
	Font_FTDep::Font_FTDep(const FontName &name, const FontId fid):
		_fontId(fid)
	{
		_hFT = mgr_font.fontFromFamilyName(name);
		// 指定したフォント名が見つからない場合は例外を投げる
		if(!_hFT)
			throw FontNotFound(lubee::MakeAssertMessage("", "font not found", name.c_str()));
		SetFTSize(*_hFT, _fontId, mgr_font.getDPI());
	}

	unsigned int Font_FTDep::height() const {
		return _hFT->getFaceInfo().height;
	}
	unsigned int Font_FTDep::width(const char32_t c) {
		auto& ft = *_hFT;
		SetFTSize(ft, _fontId, mgr_font.getDPI());
		PrepareGlyph(ft, CharId(c, _fontId));
		return ft.getGlyphInfo().advanceX;
	}
	unsigned int Font_FTDep::maxWidth() const {
		return _hFT->getFaceInfo().maxWidth;
	}
	FontId Font_FTDep::adjustParams(const FontId fid) {
		return fid;
	}
	CharData Font_FTDep::getChara(const char32_t c) {
		auto& ft = *_hFT;
		SetFTSize(ft, _fontId, mgr_font.getDPI());
		PrepareGlyph(ft, CharId(c, _fontId));
		const auto& gi = ft.getGlyphInfo();
		ByteBuff buff;
		lubee::RectI rect(gi.horiBearingX, gi.horiBearingX + gi.width,
						-gi.horiBearingY, -gi.horiBearingY + gi.height);
		if(gi.nlevel == 2)
			buff = Convert1Bit_8Bit(gi.data, gi.width, gi.pitch, gi.height);
		else {
			// nlevelが2より大きい時は256として扱う
			std::size_t sz = gi.width * gi.height;
			buff.resize(sz);
			if(gi.pitch == gi.width) {
				if(sz > 0)
					std::memcpy(buff.data(), gi.data, sz);
			} else {
				// pitchを詰める
				const uint8_t* src = gi.data;
				uint8_t* dst = buff.data();
				for(int i=0 ; i<gi.height ; i++) {
					for(int j=0 ; j<gi.width ; j++)
						std::memcpy(dst, src, gi.width);
					src += gi.pitch;
					dst += gi.width;
				}
			}
		}
		return CharData{
			.pixel = std::move(buff),
			.rect = rect
		};
	}
}
