#pragma once
#include "drawtoken_t.hpp"
#include "lubee/rect.hpp"

namespace rev {
	namespace draw {
		class Viewport : public TokenT<Viewport> {
			private:
				bool			_bPixel;
				lubee::RectF	_rect;
			public:
				Viewport(bool bPixel, const lubee::RectF& r);
				void exec() override;
		};
	}
}
