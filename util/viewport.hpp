#pragma once
#include "../drawable.hpp"
#include "lubee/rect.hpp"

namespace rev {
	namespace util {
		class Viewport : public DrawableObjT<Viewport> {
			private:
				using base_t = DrawableObjT<Viewport>;
				//! 矩形サイズ指定
				/*! true:	ピクセル
					false:	画面比率 */
				bool			_bPixel;
				lubee::RectF	_rect;
			public:
				Viewport(Priority dprio);
				void onDraw(IEffect& e) const override;
				void setByRatio(const lubee::RectF& r);
				void setByPixel(const lubee::RectF& r);
		};
	}
}
DEF_LUAIMPORT(rev::util::Viewport)
