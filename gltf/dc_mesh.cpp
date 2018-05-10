#include "gltf/dc_mesh.hpp"
#include "gltf/rt_uniform.hpp"
#include "glx_if.hpp"
#include "uniform_ent.hpp"
#include "../dc/node.hpp"
#include "gltf/node_cached.hpp"

namespace rev::gltf {
	// ------------ GLTFMesh ------------
	GLTFMesh::GLTFMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, const dc::JointId id):
		IMesh(p, t, id, userName),
		_rtParams(rt)
	{}
	GLTFMesh::GLTFMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, const dc::SkinBindV_SP& bind, const frea::Mat4& bsm):
		IMesh(p, t, 0, userName),
		_rtParams(rt),
		_bind(bind),
		_bsm(bsm)
	{}
	void GLTFMesh::draw(IEffect& e, const dc::NodeParam& np) const {
		IMesh::_applyTech(e);

		auto& npu = dynamic_cast<const NodeParam_USem&>(np);
		auto& u = e.refUniformEnt();
		{
			for(auto& rt : *_rtParams) {
				rt.second->exportUniform(u, rt.first, _jointId, _bind, _bsm, npu);
			}
		}
		IMesh::_draw(e);
	}
}
