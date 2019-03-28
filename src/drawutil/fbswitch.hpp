#pragma once
#include "../drawable.hpp"
#include "../effect/clear.hpp"

namespace rev {
	namespace drawutil {
		using ClearParam_OP = spi::Optional<ClearParam>;
		class FBSwitch : public DrawableObjT<FBSwitch> {
			private:
				using base_t = DrawableObjT<FBSwitch>;
				HFb			_hFb;
				ClearParam_OP	_cparam;
			public:
				FBSwitch(Priority dprio, const HFb& hFb, const ClearParam_OP& p=spi::none);
				void onDraw(IEffect& e) const override;
				void setClearParam(const ClearParam_OP& p);
		};
	}
}
DEF_LUAIMPORT(rev::drawutil::FBSwitch)
