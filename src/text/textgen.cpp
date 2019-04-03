#include "../resmgr_app.hpp"
#include "text.hpp"
#include <boost/lexical_cast.hpp>

namespace rev {
	using FontName_S = detail::FontName_S;
	TextGen::TextGen(const lubee::PowSize sfcSize):
		_sfcSize(sfcSize)
	{}
	FontId TextGen::appendFaceId(const FontName &name, const FontId fid) {
		return _appendFaceId(name, fid);
	}
	FontId TextGen::appendFaceId(const FontName_S &name, const FontId fid) {
		return _appendFaceId(name, fid);
	}
	detail::Face& TextGen::_getFace(const FontId fid) {
		const auto faceId = fid.at<FontId::FaceId>();
		D_Assert0(faceId != FontId::InvalidFaceId);
		// FaceList線形探索
		const auto itr = std::find_if(
			_face.begin(),
			_face.end(),
			[faceId](const detail::Face& fc){
				return fc.getFaceId() == faceId;
			}
		);
		Assert0(itr != _face.end());
		return *itr;
	}
	void TextGen::clearCache(const bool bRestore) {
		// あくまでもキャッシュのクリアなので文字列データ等は削除しない
		_fontMap.clear();
		_face.clear();
		const detail::TextObjPrivate priv;
		// 文字列クラスのキャッシュも破棄
		for(auto text : *this)
			text->onCacheLost(priv);

		if(bRestore) {
			for(auto text: *this) {
				// FaceIdは使わず再度Face参照する
				auto& c = text->refFontId(priv);
				c.at<FontId::FaceId>() = FontId::InvalidFaceId;
				// TextからSPの名前を取り出してFaceIdを更新
				c = appendFaceId(text->getFaceName(), c);
				text->onCacheReset(priv, _getFace(c));
			}
		}
	}
	std::u32string TextGen::_MakeTextTag(const FontId fid, const std::u32string &s) {
		// ハンドルキー = FontIdの64bit数値 + _ + 文字列
		std::basic_stringstream<char32_t>	ss;
		ss << boost::lexical_cast<std::u32string>(fid.value()) << U'_' << s;
		return ss.str();
	}
	HText TextGen::createText(const FontId fid, To32Str str) {
		std::u32string str32(str.moveTo());
		// FontIdを付加した文字列をキーにする
		auto& face = _getFace(fid);
		const auto tag = _MakeTextTag(fid, str32);
		return
			acquireWithMake<detail::TextObj>(
				tag,
				[&](const auto&, auto& ctor){
					ctor(face, fid, std::move(str32));
				}
			).first;
	}

	namespace {
		FontName_S ToFontName(const std::string& s) { return FontName_S(new FontName(s)); }
		FontName_S ToFontName(const FontName_S& s) { return s; }
		const std::string& ToStdString(const std::string& s) { return s; }
		const std::string& ToStdString(const FontName_S& s) { return *s; }
	}
	template <class S>
	FontId TextGen::_appendFaceId(const S &name, FontId fid) {
		auto faceId = fid.at<FontId::FaceId>();
		// FontIdのFaceIdが有効な場合は警告を出す
		Expect(faceId == FontId::InvalidFaceId,
				"FontId must have InvalidFaceId");
		// fid = フォントファミリを無視した値
		const auto itr = std::find_if(
			_face.begin(),
			_face.end(),
			[nm = ToStdString(name)](const detail::Face& f) {
				return ToStdString(f.getFaceName()) == nm;
			}
		);
		if(itr == _face.end()) {
			// 新しくFaceIdを作成
			faceId = static_cast<EnumUInt>(_face.size());
			_face.emplace_back(
				ToFontName(name),
				_sfcSize,
				faceId,
				_fontMap
			);
		} else {
			// 既存のFaceIdを記録
			faceId = itr - _face.begin();
		}
		return fid;
	}
}
