#include "gltf/dc_mesh.hpp"
#include "gltf/rt_uniform.hpp"
#include "glx_if.hpp"
#include "uniform_ent.hpp"
#include "sys_uniform.hpp"
#include "fbrect.hpp"
#include "systeminfo.hpp"

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
		auto& u = e.refUniformEnt();
		auto& td = e.ref3D();
		const auto cam = td.getCamera();
		const auto vp = e.getViewport().resolve([](){ return mgr_info.getScreenSize(); });

		for(auto& rt : *_rtParams) {
			u.setUniformWithMake(rt.first, [&](){
				return rt.second->makeToken(cam, vp, _jointId, _bind, _bsm, np);
			});
		}
		IMesh::_draw(e);
	}
}
