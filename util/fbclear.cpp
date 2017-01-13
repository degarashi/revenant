#include "fbclear.hpp"
#include "../glx_if.hpp"

namespace rev {
	namespace util {
		// --------------------- FBClear ---------------------
		FBClear::FBClear(const Priority dprio,
						const draw::ClearParam& p):
			base_t(dprio),
			_param(p)
		{}
		void FBClear::onDraw(IEffect& e) const {
			e.clearFramebuffer(_param);
		}
	}
}
