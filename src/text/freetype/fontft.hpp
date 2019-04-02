#pragma once
#include "../id.hpp"
#include "../../handle/text.hpp"
#include "../name.hpp"
#include "../chardata.hpp"
#include "lubee/src/rect.hpp"

namespace rev {
	//! フォント作成クラス: 環境依存
	/*! フォント設定(=FontId)毎に用意する */
	class Font_FTDep {
		private:
			// FTFaceとFontIdの対応
			HFT				_hFT;
			//! フォントサイズ、Italic, Bold等の設定フラグ
			FontId			_fontId;

			//! 描画に必要な範囲を取得
			lubee::RectI _boundingRect(char32_t code) const;
		public:
			Font_FTDep(Font_FTDep&&) = default;
			Font_FTDep(const FontName &name, FontId fid);

			FontId adjustParams(FontId fid);
			CharData getChara(char32_t c);
			unsigned int maxWidth() const;
			unsigned int height() const;
			unsigned int width(char32_t c);
	};
	using FontRenderer = Font_FTDep;
}
