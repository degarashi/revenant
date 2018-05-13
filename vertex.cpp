#include "vertex.hpp"

namespace rev {
	bool VSemantic::operator == (const VSemantic& v) const noexcept {
		return sem == v.sem &&
				index == v.index;
	}
	bool VSemantic::operator < (const VSemantic& v) const noexcept {
		if(sem < v.sem)
			return true;
		else if(sem > v.sem)
			return false;
		return index < v.index;
	}

	bool VSem_AttrId::operator == (const VSem_AttrId& a) const noexcept {
		return sem == a.sem &&
				attrId == a.attrId;
	}
	bool VSem_AttrId::operator < (const VSem_AttrId& a) const noexcept {
		if(sem < a.sem)
			return true;
		else if(sem == a.sem) {
			return attrId < a.attrId;
		}
		return false;
	}
}
