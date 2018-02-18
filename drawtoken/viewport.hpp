#pragma once
#include "drawtoken_t.hpp"
#include "../fbrect.hpp"

namespace rev {
	namespace draw {
		class Viewport : public TokenT<Viewport> {
			private:
				FBRect			_rect;
			public:
				Viewport(const FBRect& rect);
				void exec() override;
		};
	}
}
