#include "dc/mesh.hpp"

namespace rev::dc {
	SkinMesh::SkinMesh(const HPrim& p, const HTech& t, const Name& userName, const SkinBindV_SP& bind):
		IMesh(p, t, userName),
		_binding(bind)
	{}
	void SkinMesh::draw(IEffect&, const NodeParam&) const {
	}
}
