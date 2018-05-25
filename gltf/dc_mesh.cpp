#include "gltf/dc_mesh.hpp"
#include "gltf/rt_uniform.hpp"
#include "glx_if.hpp"
#include "uniform_ent.hpp"
#include "../dc/node.hpp"
#include "gltf/qm_usemcached.hpp"
#include "semantic_if.hpp"
#include "../tech_if.hpp"

namespace rev::gltf {
	// ------------ GLTFMesh ------------
	GLTFMesh::GLTFMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, const dc::JointId id):
		_primitive(p),
		_tech(t),
		_userName(userName),
		_rtParams(rt),
		_jointId(id)
	{
		auto& prog = *t->getProgram();
		const auto rtpLen = rt->size();
		_uId.resize(rtpLen);
		for(std::size_t i=0 ; i<rtpLen ; i++) {
			auto& r = (*rt)[i];
			if(const auto id = prog.getUniformId(r.first)) {
				_uId[i] = *id;
			} else
				_uId[i] = -1;
		}
	}
	GLTFMesh::GLTFMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, const SkinBindSet_SP& bind):
		GLTFMesh(p, t, userName, rt, 0)
	{
		_bind = bind;
	}
	void GLTFMesh::draw(IEffect& e, const IQueryMatrix_USem& qmu) const {
		e.setTechnique(_tech);
		auto& rtp = *_rtParams;
		if(!rtp.empty()) {
			struct SemanticSet : ISemanticSet {
				UniformEnt&		u;
				GLint			id;

				SemanticSet(UniformEnt& u):
					u(u)
				{}
				void set(const frea::Vec4& v) override {
					u.setUniformById(id, v);
				}
				void set(const frea::Mat3& m, const bool t) override {
					u.setUniformById(id, m, t);
				}
				void set(const frea::Mat4& m, const bool t) override {
					u.setUniformById(id, m, t);
				}
				void set(const std::vector<frea::Mat4>& mv, const bool t) override {
					u.setUniformById(id, mv, t);
				}
			};
			SemanticSet sem(e.refUniformEnt());
			const auto rtpLen = rtp.size();
			for(std::size_t i=0 ; i<rtpLen ; i++) {
				const auto uid = _uId[i];
				if(uid >= 0) {
					sem.id = uid;
					rtp[i].second->exportUniform(sem, _jointId, _bind, qmu);
				}
			}
		}
		e.setPrimitive(_primitive);
		e.draw();
	}
	HTech GLTFMesh::getTech() const {
		return _tech;
	}
}
