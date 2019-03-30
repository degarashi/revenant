//! FontCache - FreeTypeを使った実装
#pragma once
#include "wrap.hpp"
#include "../id.hpp"
#include "../../abstbuffer.hpp"
#include "../../sdl/rw.hpp"
#include "spine/src/optional.hpp"
#include "lubee/src/rect.hpp"
#include <unordered_map>

namespace rev {
	using ByteBuff = std::vector<uint8_t>;
	//! 1bitのモノクロビットマップを8bitのグレースケールへ変換
	ByteBuff Convert1Bit_8Bit(const void* src, int width, int pitch, int nrow);
	//! 8bitのグレースケールを24bitのPackedBitmapへ変換
	ByteBuff Convert8Bit_Packed24Bit(const void* src, int width, int pitch, int nrow);
	//! 8bitのグレースケールを32bitのPackedBitmapへ変換
	ByteBuff Convert8Bit_Packed32Bit(const void* src, int width, int pitch, int nrow);

	#define mgr_font static_cast<FontFamily&>(::rev::FontFamily::ref())
	//! 対象ディレクトリからフォントファイルを列挙しリストアップ
	class FontFamily : public FTLibrary {
		public:
			struct Item {
				int			faceIndex;
				HRW			hRW;
				using Path_OP = spi::Optional<std::string>;
				Path_OP		path;

				Item(Item&&) = default;
				Item(int fIdx, const HRW& hRW);
				Item(int fIdx, const std::string& p);
				HFT makeFont() const;

				Item& operator = (Item&&) = default;
			};
		private:
			//! [FamilyName -> FullPath]
			using FontMap = std::unordered_map<std::string, Item>;
			FontMap			_fontMap;

		public:
			void loadFamilyWildCard(To8Str pattern);
			void loadFamily(const HRW& hRW);

			//! FamilyNameからフォントを特定
			HFT fontFromFamilyName(const std::string& name) const;
			//! ファイル名を指定してフォントを探す
			HFT fontFromFile(const std::string& path);
			//! サイズや形式からフォントを探す
			HFT fontFromID(FontId id) const;
	};
	//! フォント作成クラス: 環境依存
	/*! フォント設定毎に用意する */
	class Font_FTDep {
		private:
			// FTFaceとFontIdの対応
			HFT				_hFT;
			// 一時バッファは対応する最大サイズで確保
			ByteBuff		_buff;
			//! フォントサイズ、Italic, Bold等の設定フラグ
			FontId			_fontId;

			//! 描画に必要な範囲を取得
			lubee::RectI _boundingRect(char32_t code) const;
		public:
			Font_FTDep(Font_FTDep&&) = default;
			Font_FTDep(const std::string& name, FontId fid);
			Font_FTDep(FontId fid);
			Font_FTDep& operator = (Font_FTDep&& dep) = default;

			//! 結果的にFontIdが同じになるパラメータの値を統一
			/*! (依存クラスによってはサイズが縦しか指定できなかったりする為) */
			FontId adjustParams(FontId fid);

			//! 使用テクスチャとUV範囲、カーソル移動距離など取得
			/*! \return first=フォントピクセルデータ(各ピクセル8bit)
						second=フォント原点に対する描画オフセット */
			std::pair<ByteBuff, lubee::RectI> getChara(char32_t c);
			int maxWidth() const;
			int height() const;
			int width(char32_t c);
	};
	using FontArray_Dep = Font_FTDep;
}
namespace std {
	template <>
	struct hash<::rev::FontFamily::Item> {
		std::size_t operator()(const ::rev::FontFamily::Item& it) const {
			if(it.hRW)
				return reinterpret_cast<std::size_t>(it.hRW.get());
			return hash<std::string>()(it.path.get());
		}
	};
}
