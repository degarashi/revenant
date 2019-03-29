#include "../resmgr_app.hpp"
#include "font.hpp"
#include <boost/lexical_cast.hpp>

namespace rev {
	using FontName_S = detail::FontName_S;
	namespace {
		FontName_S ToFontName(const std::string& s) { return FontName_S(new FontName(s)); }
		FontName_S ToFontName(const FontName_S& s) { return s; }
		const std::string& ToStdString(const std::string& s) { return s; }
		const std::string& ToStdString(const FontName_S& s) { return *s; }
	}
	FontGen::FontGen(const lubee::PowSize& sfcSize): _sfcSize(sfcSize) {}

	template <class S>
	FontId FontGen::_makeFontId(const S& name, FontId cid) {
		// cid = フォントファミリを無視した値
		const auto itr = std::find(_faceL.begin(), _faceL.end(), ToStdString(name));
		if(itr == _faceL.end()) {
			// 新しくFaceを作成
			cid.at<FontId::FaceId>() = static_cast<int>(_faceL.size());
			_faceL.emplace_back(ToFontName(name), _sfcSize, cid, _fontMap);
		} else {
			cid.at<FontId::FaceId>() = itr - _faceL.begin();
		}
		return cid;
	}

	FontId FontGen::makeFontId(const std::string& name, const FontId cid) {
		return _makeFontId(name, cid);
	}
	FontId FontGen::makeFontId(const FontName_S& name, const FontId cid) {
		return _makeFontId(name, cid);
	}
	detail::Face& FontGen::_getArray(const FontId cid) {
		// FaceList線形探索
		const auto itr = std::find_if(_faceL.begin(), _faceL.end(), [cid](const detail::Face& fc){
			return fc.fontId.at<FontId::FaceId>() == cid.at<FontId::FaceId>();
		});
		D_Assert0(itr != _faceL.end());
		return *itr;
	}
	void FontGen::clearCache(const bool bRestore) {
		// あくまでもキャッシュのクリアなので文字列データ等は削除しない
		_fontMap.clear();
		_faceL.clear();
		// 文字列クラスのキャッシュも破棄
		for(auto text : *this)
			text->onCacheLost();

		if(bRestore) {
			for(auto text: *this) {
				// FaceIdは使わず再度Face参照する
				auto& c = text->refFontId();
				// TextからSPの名前を取り出してFaceIdを更新
				c = makeFontId(text->getFaceName(), c);
				text->onCacheReset(_getArray(c));
			}
		}
	}
	std::u32string FontGen::_MakeTextTag(const FontId cid, const std::u32string& s) {
		// ハンドルキー = FontIdの64bit数値 + _ + 文字列
		std::basic_stringstream<char32_t>	ss;
		ss << boost::lexical_cast<std::u32string>(cid.value()) << U'_' << s;
		return ss.str();
	}
	HText FontGen::createText(const FontId cid, To32Str str) {
		std::u32string str32(str.moveTo());
		// FontIdを付加した文字列をキーにする
		auto& ar = _getArray(cid);
		const auto tag = _MakeTextTag(cid, str32);
		return
			acquireWithMake<detail::TextObj>(
				tag,
				[&](const auto&, auto& ctor){
					ctor(ar, cid, std::move(str32));
				}
			).first;
	}
}
