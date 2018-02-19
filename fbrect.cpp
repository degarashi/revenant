#include "fbrect.hpp"

namespace rev {
	FBRect::FBRect(const bool bPixel, const RectF& r):
		_bPixel(bPixel),
		_rect(r)
	{}
	void FBRect::setByRatio(const RectF& r) {
		_bPixel = false;
		_rect = r;
	}
	void FBRect::setByPixel(const RectF& r) {
		_bPixel = true;
		_rect = r;
	}
	lubee::RectI FBRect::resolve(const CBScr& cb) const {
		RectF r = _rect;
		if(!_bPixel) {
			const lubee::SizeI s = cb();
			r.x0 *= s.width;
			r.x1 *= s.width;
			r.y0 *= s.height;
			r.y1 *= s.height;
		}
		return r.toRect<int>();
	}
	bool FBRect::isRatio() const noexcept {
		return !_bPixel;
	}
}
