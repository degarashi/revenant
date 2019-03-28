#pragma once
#include "../drawable.hpp"
#include "../clear.hpp"

namespace rev {
	namespace drawutil {
		class FBClear : public DrawableObjT<FBClear> {
			private:
				using base_t = DrawableObjT<FBClear>;
				ClearParam _param;
			public:
				FBClear(Priority dprio,
						const ClearParam& p);
				void onDraw(IEffect& e) const override;
		};
	}
}
DEF_LUAIMPORT(rev::drawutil::FBClear)
