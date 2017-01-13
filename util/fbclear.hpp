#pragma once
#include "../drawable.hpp"
#include "../drawtoken/clear.hpp"

namespace rev {
	namespace util {
		class FBClear : public DrawableObjT<FBClear> {
			private:
				using base_t = DrawableObjT<FBClear>;
				draw::ClearParam _param;
			public:
				FBClear(Priority dprio,
						const draw::ClearParam& p);
				void onDraw(IEffect& e) const override;
		};
	}
}
DEF_LUAIMPORT(rev::util::FBClear)
