#include "dc/mesh.hpp"
#include "../glx_if.hpp"

namespace rev::dc {
	IMesh::IMesh(const HPrim& p, const HTech& t, const Name& userName):
		_primitive(p),
		_tech(t),
		_userName(userName)
	{}
	IMesh::~IMesh() {}
	const HTech& IMesh::getTech() const noexcept {
		return _tech;
	}
}
