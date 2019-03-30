#pragma once
#include "lubee/src/rect.hpp"

namespace rev {
	using ByteBuff = std::vector<uint8_t>;
	struct CharData {
		//! フォントピクセルデータ(各ピクセル8bit)
		ByteBuff		pixel;
		//! フォント原点に対する描画オフセット
		lubee::RectI	rect;
	};
}
