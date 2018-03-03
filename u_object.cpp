#include "u_object.hpp"

namespace rev {
	bool U_Object::isNode() const noexcept {
		return false;
	}
	ObjTypeId U_Object::getTypeId() const noexcept {
		return IdT::Id;
	}
}
