#include "fbswitch.hpp"
#include "viewport.hpp"
#include "../glx_if.hpp"

namespace rev {
	namespace util {
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
			Viewport(0x0000).onDraw(e);
		}
	}
}
