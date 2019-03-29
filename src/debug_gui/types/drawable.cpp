#include "../../object/drawable.hpp"
#include "../tree.hpp"
#include "../sstream.hpp"
#include <iomanip>

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
	std::string DrawableObj::summary_str() const {
		StringStream s;
		s << std::hex;
		s << "0x" << std::setfill('0') << std::setw(8) << _dtag.priority;
		if(this->isNode())
			s << " [G]";
		if(this->isDead())
			s << " [D]";
		s << "\t" << getDebugName();
		return s.output();
	}
}
