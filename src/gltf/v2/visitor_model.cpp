#include "gltf/v2/visitor_model.hpp"
#include "gltf/v2/dc_model.hpp"
#include "../../dc/node.hpp"

namespace rev::gltf::v2 {
	Visitor_Model::Visitor_Model():
		_tfRoot(std::make_shared<dc::TfRoot>())
	{}
	void Visitor_Model::upNode() {
		_stack.pop_back();
	}
	void Visitor_Model::addNode(const Node& n) {
		auto node = std::make_shared<dc::TfNode>(n.jointId, SName(), n.getName());
		node->userData = n.userData;
		node->setPose(n.pose);
		if(_stack.empty()) {
			_tfRoot->refNode().emplace_back(node);
		} else {
			_stack.back()->addChild(node);
		}
		_stack.emplace_back(node.get());
	}
	void Visitor_Model::addMesh(const HGMesh& m) {
		_mesh.emplace_back(m);
	}
	void Visitor_Model::addSkinMesh(const HGMesh& m) {
		_skinmesh.emplace_back(m);
	}
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
