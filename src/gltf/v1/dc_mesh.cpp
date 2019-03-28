#include "gltf/v1/dc_mesh.hpp"
#include "gltf/v1/rt_uniform.hpp"
#include "gltf/v1/qm_usemcached.hpp"
#include "semantic_if.hpp"
#include "../../effect/if.hpp"
#include "../../effect/uniform_ent.hpp"
#include "../../effect/tech_if.hpp"
#include "../../dc/node.hpp"
#include "../../drawcmd/aux.hpp"

namespace rev::gltf::v1 {
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
	bool GLTFMesh::draw(IEffect& e, const IQueryMatrix_USem& qmu) const {
		bool ret = false;
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

			if(!_flip) {
				const auto m = qmu.getGlobal(_jointId);
				_flip = m.calcDeterminant() > 0;
				ret = true;
			}
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

		if(*_flip)
			e.refQueue().add(draw::FlipFace());
		e.draw();
		if(*_flip)
			e.refQueue().add(draw::FlipFace());
		return ret;
	}
	HTech GLTFMesh::getTech() const {
		return _tech;
	}
}
