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
	FontGen::FontGen(const lubee::PowSize sfcSize):
		_sfcSize(sfcSize)
	{}

	template <class S>
	FontId FontGen::_makeFontId(const S &name, FontId fid) {
		// fid = フォントファミリを無視した値
		const auto itr = std::find(_faceL.begin(), _faceL.end(), ToStdString(name));
		if(itr == _faceL.end()) {
			// 新しくFaceを作成
			fid.at<FontId::FaceId>() = static_cast<EnumUInt>(_faceL.size());
			_faceL.emplace_back(ToFontName(name), _sfcSize, fid, _fontMap);
		} else {
			fid.at<FontId::FaceId>() = itr - _faceL.begin();
		}
		return fid;
	}

	FontId FontGen::makeFontId(const FontName &name, const FontId fid) {
		return _makeFontId(name, fid);
	}
	FontId FontGen::makeFontId(const FontName_S &name, const FontId fid) {
		return _makeFontId(name, fid);
	}
	detail::Face& FontGen::_getFace(const FontId fid) {
		// FaceList線形探索
		const auto itr = std::find_if(
			_faceL.begin(),
			_faceL.end(),
			[fid](const detail::Face& fc){
				return fc.fontId.at<FontId::FaceId>() == fid.at<FontId::FaceId>();
			}
		);
		Assert0(itr != _faceL.end());
		return *itr;
	}
	void FontGen::clearCache(const bool bRestore) {
		// あくまでもキャッシュのクリアなので文字列データ等は削除しない
		_fontMap.clear();
		_faceL.clear();
		const detail::TextObjPrivate priv;
		// 文字列クラスのキャッシュも破棄
		for(auto text : *this)
			text->onCacheLost(priv);

		if(bRestore) {
			for(auto text: *this) {
				// FaceIdは使わず再度Face参照する
				auto& c = text->refFontId(priv);
				// TextからSPの名前を取り出してFaceIdを更新
				c = makeFontId(text->getFaceName(), c);
				text->onCacheReset(priv, _getFace(c));
			}
		}
	}
	std::u32string FontGen::_MakeTextTag(const FontId fid, const std::u32string &s) {
		// ハンドルキー = FontIdの64bit数値 + _ + 文字列
		std::basic_stringstream<char32_t>	ss;
		ss << boost::lexical_cast<std::u32string>(fid.value()) << U'_' << s;
		return ss.str();
	}
	HText FontGen::createText(const FontId fid, To32Str str) {
		std::u32string str32(str.moveTo());
		// FontIdを付加した文字列をキーにする
		auto& ar = _getFace(fid);
		const auto tag = _MakeTextTag(fid, str32);
		return
			acquireWithMake<detail::TextObj>(
				tag,
				[&](const auto&, auto& ctor){
					ctor(ar, fid, std::move(str32));
				}
			).first;
	}
}
