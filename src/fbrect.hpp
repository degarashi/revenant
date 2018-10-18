#pragma once
#include "lubee/src/rect.hpp"
#include "lubee/src/size.hpp"

namespace rev {
	class FBRect {
		private:
			using RectI = lubee::RectI;
			using RectF = lubee::RectF;
			using SizeI = lubee::SizeI;
			//! 矩形サイズ指定
			/*! true:	ピクセル
				false:	画面比率 */
			bool		_bPixel;
			RectF		_rect;
			using CBScr = std::function<SizeI ()>;
		public:
			FBRect(bool bPixel, const RectF& r);
			void setByRatio(const RectF& r);
			void setByPixel(const RectF& r);
			RectI resolve(const CBScr& cb) const;
			bool isRatio() const noexcept;
	};
}
