#pragma once
#include "uniform.hpp"
#include "../fbrect.hpp"

namespace rev {
	namespace draw {
		class Scissor : public TokenT<Scissor> {
			private:
				FBRect			_rect;
			public:
				Scissor(const FBRect& rect);
				void exec() override;
		};
	}
}
