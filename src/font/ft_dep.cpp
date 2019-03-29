#include "ft_dep.hpp"
#include "../fs/dir.hpp"
#include "emplace.hpp"
#include "lubee/src/error.hpp"

namespace rev {
	// ---------------------- FontFamily ----------------------
	FontFamily::Item::Item(const int fIdx, const HRW& hRW):
		faceIndex(fIdx),
		hRW(hRW)
	{}
	FontFamily::Item::Item(const int fIdx, const std::string& p):
		faceIndex(fIdx),
		path(p)
	{}
	HFT FontFamily::Item::makeFont() const {
		if(hRW)
			return mgr_font.newFace(hRW, faceIndex);
		return mgr_font.newFace(mgr_rw.fromFile(PathBlock(*path), Access::Read|Access::Binary), faceIndex);
	}
	void FontFamily::loadFamilyWildCard(To8Str pattern) {
		std::size_t len = pattern.getLength();
		if(len == 0)
			return;
		Dir::EnumEntryWildCard(pattern.moveTo(), [this](const Dir& dir) {
			loadFamily(mgr_rw.fromFile(dir, Access::Read|Access::Binary));
		});
	}

	void FontFamily::loadFamily(const HRW& hRW) {
		HFT hlf = newFace(hRW, 0);
		auto& face = *hlf;
		const int nf = face.getNFace();
		EmplaceOrReplace(_fontMap, face.getFamilyName(), 0, hRW);
		for(int i=1 ; i<nf ; i++) {
			HFT hf2 = newFace(hRW, i);
			EmplaceOrReplace(_fontMap, hf2->getFamilyName(), i, hRW);
		}
	}
	HFT FontFamily::fontFromFamilyName(const std::string& name) const {
		auto itr = _fontMap.find(name);
		if(itr == _fontMap.end())
			return HFT();
		return itr->second.makeFont();
	}
	HFT FontFamily::fontFromFile(const std::string& /*path*/) {
		AssertF("not implemented yet");
		return HFT();
	}
	HFT FontFamily::fontFromID(FontId /*id*/) const {
		AssertF("not implemented yet");
		return HFT();
	}

	// ---------------------- Font_FTDep ----------------------
	namespace {
		void SetFTSize(FTFace& ft, const FontId fontId) {
			// TODO: この値は埋め込むのではなくてディスプレイから取得するべき
			constexpr int DPI_W = 300,
						DPI_H = 300;
			int w = fontId.at<FontId::Width>(),
				h = fontId.at<FontId::Height>();
			switch(fontId.at<FontId::SizeType>()) {
				case FontId::SizeType_Pixel:
					ft.setPixelSizes(w, h); break;
				case FontId::SizeType_Point:
					ft.setCharSize(w, h, DPI_W, DPI_H); break;
				case FontId::SizeType_LineHeight:
					ft.setSizeFromLine(h); break;
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
	Font_FTDep::Font_FTDep(const std::string& name, const FontId cid):
		_fontId(cid)
	{
		_hFT = mgr_font.fontFromFamilyName(name);
		Assert0(_hFT);
		SetFTSize(*_hFT, _fontId);
	}
	Font_FTDep::Font_FTDep(const FontId cid):
		_fontId(cid)
	{
		_hFT = mgr_font.fontFromID(cid);
		Assert0(_hFT);
		SetFTSize(*_hFT, _fontId);
	}

	int Font_FTDep::height() const {
		return _hFT->getFaceInfo().height;
	}
	int Font_FTDep::width(const char32_t c) {
		auto& ft = *_hFT;
		SetFTSize(ft, _fontId);
		PrepareGlyph(ft, CharId(c, _fontId));
		return ft.getGlyphInfo().advanceX;
	}
	int Font_FTDep::maxWidth() const {
		return _hFT->getFaceInfo().maxWidth;
	}
	FontId Font_FTDep::adjustParams(const FontId cid) {
		return cid;
	}
	std::pair<ByteBuff, lubee::RectI> Font_FTDep::getChara(const char32_t c) {
		auto& ft = *_hFT;
		SetFTSize(ft, _fontId);
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
		return std::make_pair(std::move(buff), rect);
	}
	ByteBuff Convert1Bit_8Bit(const void* src, const int width, const int pitch, const int nrow) {
		auto* pSrc = reinterpret_cast<const uint8_t*>(src);
		ByteBuff buff(width * nrow);
		auto* dst = buff.data();
		for(int i=0 ; i<nrow ; i++) {
			for(int j=0 ; j<width ; j++)
				*dst++ = ((int32_t(0) - (pSrc[j/8] & (1<<(7-(j%8)))))>>8) & 0xff;
			pSrc += pitch;
		}
		return buff;
	}
	namespace {
		template <int NB>
		ByteBuff ExpandBits(const void* src, const int width, const int pitch, const int nrow) {
			auto* pSrc = reinterpret_cast<const uint8_t*>(src);
			ByteBuff buff(width*nrow*NB);
			auto* dst = reinterpret_cast<uint8_t*>(buff.data());
			for(int i=0 ; i<nrow ; i++) {
				for(int j=0 ; j<width ; j++) {
					auto tmp = pSrc[j];
					for(int k=0 ; k<NB ; k++)
						*dst++ = tmp;
				}
				pSrc += pitch;
			}
			return buff;
		}
	}
	ByteBuff Convert8Bit_Packed24Bit(const void* src, const int width, const int pitch, const int nrow) {
		return ExpandBits<3>(src, width, pitch, nrow);
	}
	ByteBuff Convert8Bit_Packed32Bit(const void* src, const int width, const int pitch, const int nrow) {
		return ExpandBits<4>(src, width, pitch, nrow);
	}
}
