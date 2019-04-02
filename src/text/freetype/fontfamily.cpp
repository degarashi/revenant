#include "fontfamily.hpp"
#include "../../fs/dir.hpp"
#include "../../emplace.hpp"
#include "../../sdl/rw.hpp"

namespace rev {
	FontFamily::FontFamily(const lubee::SizeI dpi):
		_dpi(dpi)
	{}
	lubee::SizeI FontFamily::getDPI() const noexcept {
		return _dpi;
	}
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
	HFT FontFamily::fontFromFamilyName(const FontName &name) const {
		auto itr = _fontMap.find(name);
		if(itr == _fontMap.end())
			return HFT();
		return itr->second.makeFont();
	}
}
