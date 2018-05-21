#include "gltf/dc_mesh.hpp"
#include "gltf/rt_uniform.hpp"
#include "glx_if.hpp"
#include "uniform_ent.hpp"
#include "../dc/node.hpp"
#include "gltf/node_cached.hpp"
#include "semantic_if.hpp"

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
		{
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
				void set(const std::vector<frea::Mat4>& mv, const bool) override {
					u.setUniformById(id, mv);
				}
			};
			auto& u = e.refUniformEnt();
			SemanticSet sem(u);
			auto& prog = u.getProgram();
			auto& npu = dynamic_cast<const NodeParam_USem&>(np);
			for(auto& rt : *_rtParams) {
				if(const auto id = prog.getUniformId(rt.first)) {
					sem.id = *id;
					rt.second->exportUniform(sem, _jointId, _bind, npu);
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
