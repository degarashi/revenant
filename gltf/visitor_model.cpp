#include "gltf/visitor_model.hpp"
#include "gltf/dc_mesh.hpp"
#include "../dc/node.hpp"
#include "../dc/model.hpp"

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
	void Visitor_Model::addSkinMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, const dc::SkinBindV_SP& bind, const frea::Mat4& bsm) {
		_mesh.emplace_back(new GLTFMesh(p, t, userName, rt, bind, bsm));
	}
	void Visitor_Model::addCamera(const HCam3&) {}
	HMdl Visitor_Model::result() const {
		return std::make_shared<dc::Model>(_mesh, _tfRoot);
	}
}
