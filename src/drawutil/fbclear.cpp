#include "fbclear.hpp"
#include "../glx_if.hpp"

namespace rev {
	namespace drawutil {
		// --------------------- FBClear ---------------------
		FBClear::FBClear(const Priority dprio,
						const ClearParam& p):
			base_t(dprio),
			_param(p)
		{}
		void FBClear::onDraw(IEffect& e) const {
			e.clearFramebuffer(_param);
		}
	}
}
