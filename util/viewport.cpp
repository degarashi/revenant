#include "viewport.hpp"
#include "../glx_if.hpp"

namespace rev {
	namespace util {
		// --------------------- Viewport ---------------------
		Viewport::Viewport(const Priority dprio):
			base_t(dprio)
		{
			setByRatio({0,1,0,1});
		}
		void Viewport::setByRatio(const lubee::RectF& r) {
			_bPixel = false;
			_rect = r;
		}
		void Viewport::setByPixel(const lubee::RectF& r) {
			_bPixel = true;
			_rect = r;
		}
		void Viewport::onDraw(IEffect& e) const {
			e.setViewport(_bPixel, _rect);
		}
	}
}
