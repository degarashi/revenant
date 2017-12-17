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
}
