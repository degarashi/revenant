#include "gltf/dc_mesh.hpp"
#include "gltf/rt_uniform.hpp"
#include "glx_if.hpp"
#include "uniform_ent.hpp"
#include "../dc/node.hpp"
#include "gltf/node_cached.hpp"

namespace rev::gltf {
	// ------------ GLTFMesh ------------
	GLTFMesh::GLTFMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, const dc::JointId id):
		_primitive(p),
		_tech(t),
		_userName(userName),
		_rtParams(rt),
		_jointId(id)
	{}
	GLTFMesh::GLTFMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, const dc::SkinBindSet_SP& bind):
		GLTFMesh(p, t, userName, rt, 0)
	{
		_bind = bind;
	}
	void GLTFMesh::draw(IEffect& e, const dc::NodeParam& np) const {
		e.setTechnique(_tech);
		auto& npu = dynamic_cast<const NodeParam_USem&>(np);
		auto& u = e.refUniformEnt();
		{
			for(auto& rt : *_rtParams) {
				rt.second->exportUniform(u, rt.first, _jointId, _bind, npu);
			}
		}
		e.setPrimitive(_primitive);
		e.draw();
	}
	HTech GLTFMesh::getTech() const {
		return _tech;
	}
}
