#include "../resmgr_app.hpp"
#include "font.hpp"
#include <boost/lexical_cast.hpp>

namespace rev {
	namespace {
		String_SP ToSp(const std::string& s) { return String_SP(new FontName(s)); }
		String_SP ToSp(const String_SP& s) { return s; }
		const std::string& ToCp(const std::string& s) { return s; }
		const std::string& ToCp(const String_SP& s) { return *s; }
	}
	FontGen::FontGen(const lubee::PowSize& sfcSize): _sfcSize(sfcSize) {}

	template <class S>
	CCoreID FontGen::_makeCoreID(const S& name, CCoreID cid) {
		// cid = フォントファミリを無視した値
		auto itr = std::find(_faceL.begin(), _faceL.end(), ToCp(name));
		if(itr == _faceL.end()) {
			// 新しくFaceを作成
			cid.at<CCoreID::FaceID>() = static_cast<int>(_faceL.size());
			_faceL.emplace_back(ToSp(name), _sfcSize, cid, _fontMap);
		} else {
			cid.at<CCoreID::FaceID>() = itr - _faceL.begin();
		}
		return cid;
	}

	CCoreID FontGen::makeCoreID(const std::string& name, CCoreID cid) {
		return _makeCoreID(name, cid);
	}
	CCoreID FontGen::makeCoreID(const String_SP& name, CCoreID cid) {
		return _makeCoreID(name, cid);
	}
	Face& FontGen::_getArray(CCoreID cid) {
		// FaceList線形探索
		auto itr = std::find_if(_faceL.begin(), _faceL.end(), [cid](const Face& fc){
			return fc.coreID.at<CCoreID::FaceID>() == cid.at<CCoreID::FaceID>();
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
				// FaceIDは使わず再度Face参照する
				auto& c = text->refCoreID();
				// TextからSPの名前を取り出してFaceIDを更新
				c = makeCoreID(text->getFaceName(), c);
				text->onCacheReset(_getArray(c));
			}
		}
	}
	std::u32string FontGen::_MakeTextTag(const CCoreID cid, const std::u32string& s) {
		// ハンドルキー = CCoreIDの64bit数値 + _ + 文字列
		std::basic_stringstream<char32_t>	ss;
		ss << boost::lexical_cast<std::u32string>(cid.value()) << U'_' << s;
		return ss.str();
	}
	HText FontGen::createText(const CCoreID cid, To32Str str) {
		std::u32string str32(str.moveTo());
		// CCoreIDを付加した文字列をキーにする
		auto& ar = _getArray(cid);
		const auto tag = _MakeTextTag(cid, str32);
		return
			acquireWithMake<TextObj>(
				tag,
				[&](const auto&, auto& ctor){
					ctor(ar, cid, std::move(str32));
				}
			).first;
	}
}
