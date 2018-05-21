#include "gltf/visitor_model.hpp"
#include "gltf/dc_mesh.hpp"
#include "gltf/dc_model.hpp"
#include "../dc/node.hpp"
#include "../dc/model_if.hpp"

namespace rev::gltf {
	Visitor_Model::Visitor_Model():
		_tfRoot(std::make_shared<dc::TfRoot>())
	{}
	void Visitor_Model::upNode() {
		_stack.pop_back();
	}
	void Visitor_Model::addNode(const HTfNode& node) {
		if(_stack.empty()) {
			_tfRoot->refNode().emplace_back(node);
		} else {
			_stack.back()->addChild(node);
		}
		_stack.emplace_back(node.get());
	}
	void Visitor_Model::addMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, const dc::JointId id) {
		_mesh.emplace_back(new GLTFMesh(p, t, userName, rt, id));
	}
	void Visitor_Model::addSkinMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, const dc::SkinBindSet_SP& bind) {
		_skinmesh.emplace_back(new GLTFMesh(p, t, userName, rt, bind));
	}
	void Visitor_Model::addCamera(const HCam3&) {}
	HMdl Visitor_Model::result() const {
		auto* self = const_cast<Visitor_Model*>(this);
		// メッシュをTechでソート
		const auto sortMesh = [](auto& m){
			std::sort(m.begin(), m.end(), [](const auto& m0, const auto& m1){
				return m0->getTech().get() < m1->getTech().get();
			});
		};
		sortMesh(self->_mesh);
		sortMesh(self->_skinmesh);
		return std::make_shared<GLTFModel>(_mesh, _skinmesh, _tfRoot);
	}
}
