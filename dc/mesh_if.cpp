#include "dc/mesh.hpp"
#include "../glx_if.hpp"

namespace rev::dc {
	IMesh::IMesh(const HPrim& p, const HTech& t, const JointId id, const Name& userName):
		_primitive(p),
		_tech(t),
		_jointId(id),
		_userName(userName)
	{}
	IMesh::~IMesh() {}
	void IMesh::_draw(IEffect& e) const {
		e.setPrimitive(_primitive);
		e.draw();
	}
	void IMesh::_applyTech(IEffect& e) const {
		e.setTechnique(_tech);
	}
	const HTech& IMesh::getTech() const noexcept {
		return _tech;
	}
}
