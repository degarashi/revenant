#include "fbswitch.hpp"
#include "../effect/if.hpp"
#include "../fbrect.hpp"
#include "lubee/src/rect.hpp"

namespace rev {
	namespace drawutil {
		// --------------------- FBSwitch ---------------------
		FBSwitch::FBSwitch(const Priority dprio, const HFb& hFb, const ClearParam_OP& cp):
			base_t(dprio),
			_hFb(hFb),
			_cparam(cp)
		{}
		void FBSwitch::setClearParam(const ClearParam_OP& p) {
			_cparam = p;
		}
		// これ自体の描画はしない
		void FBSwitch::onDraw(IEffect& e) const {
			if(_hFb)
				e.setFramebuffer(_hFb);
			else
				e.resetFramebuffer();
			if(_cparam)
				e.clearFramebuffer(*_cparam);

			// ビューポートはフルスクリーンで初期化
			e.setViewport({false, {0,1,0,1}});
		}
	}
}
