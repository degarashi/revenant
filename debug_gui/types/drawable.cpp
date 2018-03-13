#include "../../drawable.hpp"
#include "../tree.hpp"

namespace rev {
	const char* DrawableObj::getDebugName() const noexcept {
		return "DrawableObj";
	}
	bool DrawableObj::property(const bool edit) {
		bool mod = false;
		mod |= _dtag.property(edit);
		if(const auto _ = debug::TreePush(IObject::getDebugName()))
			mod |= IObject::property(edit);
		return mod;
	}
}
